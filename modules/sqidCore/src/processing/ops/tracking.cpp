/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "tracking.h"

#include "../../drawing/frameDrawerTracking.h"

#include <fileIO/json.h>

#include <commonImGui.h>


namespace sqid
{
	namespace Tracking
	{
		DEFINE_OP_DESC( BlobTracker, "blobTracker", "/imaging/tracking",
			"1BCE6789-2B97-48E5-AD26-3013EC45B3EE" );


		BlobTracker::BlobTracker() :
			Op(),
			_minThreshold( 0 ),
			_maxThreshold( 75 ),
			_posWeight( 1.0f ),
			_sizeWeight( 0.1f ),
			_angleWeight( 0.0f ),
			_confidenceTheshConfirmed( 0.9f ),
			_confidenceTheshLost( 0.5f ),
			_confidenceTheshDiscard( 0.1f ),
			_trackCntr( 0 )
		{
			reinitialize();
		}

		BlobTracker::~BlobTracker()
		{
			clear();
		}

		void BlobTracker::reinitialize()
		{
			// Setup SimpleBlobDetector parameters.
			cv::SimpleBlobDetector::Params params;

			// Change thresholds
			params.minThreshold = _minThreshold;
			params.maxThreshold = _maxThreshold;

			params.filterByColor = true;
			params.blobColor = 255;

			// Filter by Area.
			params.filterByArea = false;
			params.minArea = 15;

			// Filter by Circularity
			params.filterByCircularity = true;
			params.minCircularity = 0.1f;

			// Filter by Convexity
			params.filterByConvexity = false;
			params.minConvexity = 0.87f;

			// Filter by Inertia
			params.filterByInertia = true;
			params.minInertiaRatio = 0.01f;

			params.minRepeatability = 1;

			_detector = cv::SimpleBlobDetector::create( params );

			clear();
		}

		void BlobTracker::clear()
		{
			for( auto &it : _tracks )
				safeDelete( it );
			_tracks.clear();
		}

#ifdef __SUPPORT_GUI
		bool BlobTracker::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			bool init = false;

			init |= ImGui::SliderInt( "thresh min", &_minThreshold, 0, 255 );
			init |= ImGui::SliderInt( "thresh max", &_maxThreshold, 0, 255 );

			if( _minThreshold > _maxThreshold )
			{
				_minThreshold = _maxThreshold;
				init = true;
			}
			if( _maxThreshold < _minThreshold )
			{
				_maxThreshold = _minThreshold;
				init = true;
			}

			if( init )
				reinitialize();

			return true;
		}

		std::vector<FrameDrawer*> BlobTracker::createDrawers()
		{
			std::vector<FrameDrawer*> drawers;

			drawers.push_back( new FrameDrawerBlob( this ) );

			return drawers;
		}
#endif

		bool BlobTracker::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				std::vector<cv::KeyPoint> keypoints;

				cv::Mat m( sf->width(), sf->height(), CV_8UC1 );
				sf->mat().convertTo( m, CV_8UC1 );

				_detector->detect( m, keypoints );

				std::vector<State> observations;
				for( auto k : keypoints )
					observations.push_back( State( cv2glm( k.pt ), k.size, k.angle ) );
				float time = sf->timeStamp() * 0.001f;

				//predict
				std::vector<State> pred;
				for( auto &it : _tracks )
					pred.push_back( it->predict( time ) );

				//assign
				std::vector<Track*> unassigned;
				std::vector<uint> unknown;
				std::vector<std::pair<uint, Track*>> assignments;
				assignGreedy( observations, _tracks, pred, assignments, unassigned, unknown, _posWeight, _sizeWeight, _angleWeight );

				//correct observations
				for( auto &it : assignments )
					it.second->insert( observations[it.first], time );

				//extrapolate tracks with missing observations
				for( auto &it : unassigned )
					it->extrapolate( time );

				//update tracking states 
				for( auto &it : _tracks )
					switch( it->_trackingState )
					{
					case TS_TENTATIVE:
						if( it->_confidence > _confidenceTheshConfirmed )
							it->_trackingState = TS_CONFIRMED;
						else if( it->_confidence < _confidenceTheshDiscard )
							it->_trackingState = TS_DISCARDED;
						break;
					case TS_CONFIRMED:
						if( it->_confidence < _confidenceTheshLost )
							it->_trackingState = TS_LOST;
						break;
					case TS_LOST:
						if( it->_confidence > _confidenceTheshConfirmed )
							it->_trackingState = TS_CONFIRMED;
						else if( it->_confidence < _confidenceTheshDiscard )
							it->_trackingState = TS_DISCARDED;
						break;
					case TS_DISCARDED:
						break;
					default:
						std::cerr << "<error> unknown tracking state" << std::endl;
					}

				//prune lost tracks
				{
					auto it = _tracks.begin();
					while( it != _tracks.end() )
					{
						if( ( *it )->_trackingState == TS_DISCARDED )
						{
							safeDelete( *it );
							it = _tracks.erase( it );
						}
						else
							it++;
					}
				}

				//create new tracks from unknown observations
				for( auto &it : unknown )
				{
					const cv::KeyPoint &k = keypoints[it];
					Track *track = new Track( _trackCntr++, State( cv2glm( k.pt ), k.size, k.angle ), time );
					_tracks.push_back( track );
				}

				//update blobs and stats of all tracks
				for( auto &it : _tracks )
					it->update();

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool BlobTracker::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<int>( j, "minThreshold", _minThreshold );
			load<int>( j, "maxThreshold", _maxThreshold );

			return ret;
		}

		bool BlobTracker::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "minThreshold", _minThreshold );
			save( j, "maxThreshold", _maxThreshold );

			return ret;
		}

		std::vector<const BlobTracker::Blob*> BlobTracker::getBlobs() const
		{
			std::vector<const Blob*> ret;

			for( auto &it : _tracks )
				ret.push_back( &it->_blob );

			return ret;
		}

		bool BlobTracker::assignGreedy( const std::vector<State> &observations, const std::list<Track*> &tracks, const std::vector<State> &pred, std::vector<std::pair<uint, Track*> > &assignments, std::vector<Track*> &unassigned, std::vector<uint> &unknown, float weightPos, float weightSize, float weightAngle )
		{
			assignments.clear();
			unassigned.clear();
			unknown.clear();

			if( !tracks.size() )
			{
				for( int i = 0; i < observations.size(); i++ )
					unknown.push_back( i );
				return true;
			}

			//NOTE: very naiive implementation of greedy assignment -> use greedy exchange to optimize

			std::vector<bool> taken( tracks.size() );
			std::vector<State> states;
			for( auto &it : tracks )
				states.push_back( it->getState() );

			for( int j = 0; j < observations.size(); j++ )
			{
				int minPos = -1;
				float minErr = 0;
				Track *minTrack = nullptr;

				int i = 0;
				for( auto it = tracks.begin(); it != tracks.end(); ++it, i++ )
				{
					if( taken[i] )
						continue;

					float err = states[i].errSquared( observations[j], weightPos, weightSize, weightAngle );
					if( err < minErr || !minTrack )
					{
						minPos = i;
						minErr = err;
						minTrack = *it;
					}
				}

				if( !minTrack )
				{
					unknown.push_back( j );
					continue;
				}

				assignments.push_back( std::make_pair( j, minTrack ) );
				taken[minPos] = true;
			}

			int i = 0;
			for( auto it = tracks.begin(); it != tracks.end(); ++it, i++ )
				if( !taken[i] )
					unassigned.push_back( *it );

			return true;
		}
	}
}