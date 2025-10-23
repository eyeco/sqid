/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <processing/op.h>

namespace sqid
{
	namespace Tracking
	{
		class BlobTracker : public Op
		{
		public:
			enum TrackingState
			{
				TS_TENTATIVE,
				TS_CONFIRMED,
				TS_LOST,
				TS_DISCARDED,

				TS_COUNT
			};

			struct Blob
			{
				const uint id;

				glm::vec2 pos;
				float size;
				float angle;

				TrackingState trackingState;

				std::vector<glm::vec2> history;

				explicit Blob( uint id ) :
					id( id ),
					pos( 0 ),
					size( 0.0f ),
					angle( 0.0f ),
					trackingState( TS_TENTATIVE )
				{}
			};

		private:
			template<typename T>
			struct Feature
			{
				T _p;
				T _v;
				T _a;

				T _posMean;
				T _posSD;

				float _weight;

				float _time;

				std::list<T> _trajectory;
				uint _trajectoryMaxLength;

				Feature( float weight, uint trajectoryMaxLength ) :
					_weight( weight ),
					_trajectoryMaxLength( trajectoryMaxLength )
				{}

				void add( const T &p, float time )
				{
					if( _trajectory.size() > 0 )
					{
						float dtInv = 1.0f / ( time - _time );

						T v = (T) ( ( p - _p ) * dtInv );
						if( _trajectory.size() > 1 )
							_a = ( v - _v ) * dtInv;
						_v = v;
					}
					_p = p;
					_time = time;

					_trajectory.push_back( p );
					while( _trajectory.size() > _trajectoryMaxLength )
						_trajectory.pop_front();
				}

				const T &extrapolate( float time )
				{
					float dt = ( time - _time );

					_p = _v * dt;
					_v = _a * dt;

					_time = time;

					_trajectory.push_back( _p );
					while( _trajectory.size() > _trajectoryMaxLength )
						_trajectory.pop_front();

					return _p;
				}

				T predict( float time ) const
				{
					//NOTE: first naiive implementation, maybe use multi-hypothesis or interacting multi model filter
					//TODO: handle cases with sudden high lag, e.g. use threshold
					return _p + _v * ( time - _time );
				}

				void updateStats()
				{
					T mean = (T) ( 0 );
					T SD = (T) ( 0 );
					if( _trajectory.size() )
					{
						float f = 1.0f / _trajectory.size();
						T sum = (T) ( 0 );
						for( auto &it : _trajectory )
							sum += it;
						mean = sum * f;

						T temp = (T) ( 0 );
						sum = (T) ( 0 );
						for( auto &it : _trajectory )
						{
							temp = ( it - mean );
							sum += temp * temp;
						}
						SD = sqrt( sum * f );
					}
					_posMean = mean;
					_posSD = SD;
				}
			};

			struct State
			{
				const glm::vec2 _pos;
				const float _size;
				const float _angle;

				State( const State &rhs ) :
					_pos( rhs._pos ),
					_size( rhs._size ),
					_angle( rhs._angle )
				{}

				State( const glm::vec2 &pos, float size, float angle ) :
					_pos( pos ),
					_size( size ),
					_angle( angle )
				{}

				State operator = ( const State &rhs )
				{
					return State( rhs );
				}

				float errSquared( const State &s, float wPos, float wSize, float wAngle ) const
				{
					return norm2( _pos - s._pos ) * wPos +
						sqr( _size - s._size ) * wSize +
						sqr( _angle - s._angle ) * wAngle;
				}

				float err( const State &s, float wPos, float wSize, float wAngle ) const
				{
					return norm( _pos - s._pos ) * wPos +
						std::abs( _size - s._size ) * wSize +
						std::abs( _angle - s._angle ) * wAngle;
				}
			};

			struct Track
			{
				const uint _id;
				float _confidence;

				Feature<glm::vec2> _pos;
				Feature<float> _size;
				Feature<float> _angle;

				TrackingState _trackingState;

				const float _confidenceDrop;
				const float _confidenceRise;

				const float _confidenceStep;

				Blob _blob;

				Track( uint id, State state, float time ) :
					_id( id ),
					_confidence( 0.5f ),
					_pos( 1.0f, 64 ),
					_size( 1.0f, 64 ),
					_angle( 1.0f, 64 ),
					_trackingState( TS_TENTATIVE ),
					_confidenceDrop( 0.75f ),
					_confidenceRise( 0.75f ),
					_confidenceStep( 0.0001f ),
					_blob( id )
				{
					insert( state, time );
				}

				void insert( const State &state, float time )
				{
					_pos.add( state._pos, time );
					_size.add( state._size, time );
					_angle.add( state._angle, time );

					float t = time;
					while( t > 0 )
					{
						_confidence += ( 1.0f - _confidence ) * _confidenceRise * sqid::min( _confidenceStep, t );
						t -= _confidenceStep;
					}
				}

				State getState() const
				{
					return State(
						_pos._p,
						_size._p,
						_angle._p );
				}

				State predict( float time ) const
				{
					return State(
						_pos.predict( time ),
						_size.predict( time ),
						_angle.predict( time ) );
				}

				void extrapolate( float time )
				{
					_pos.extrapolate( time );
					_size.extrapolate( time );
					_angle.extrapolate( time );

					float t = time;
					while( t > 0 )
					{
						_confidence -= _confidence * _confidenceDrop * sqid::min( _confidenceStep, t );
						t -= _confidenceStep;
					}
				}

				void update()
				{
					_pos.updateStats();
					_size.updateStats();
					_angle.updateStats();

					//TODO: add filtered instead of raw observational data here
					_blob.pos = _pos._p;
					_blob.size = _size._p;
					_blob.angle = _angle._p;
					_blob.history.push_back( _pos._p );
					_blob.trackingState = _trackingState;
				}
			};

			int _minThreshold;
			int _maxThreshold;

			cv::Ptr<cv::SimpleBlobDetector> _detector;

			float _posWeight;
			float _sizeWeight;
			float _angleWeight;

			float _confidenceTheshConfirmed;
			float _confidenceTheshLost;
			float _confidenceTheshDiscard;

			uint _trackCntr;
			std::list<Track*> _tracks;

			static bool assignGreedy( const std::vector<State> &observations, const std::list<Track*> &tracks, const std::vector<State> &pred, std::vector<std::pair<uint, Track*> > &assignments, std::vector<Track*> &unassigned, std::vector<uint> &unknown, float weightPos, float weightSize, float weightAngle );

		protected:
			virtual bool process();

			void clear();
			void reinitialize();

		public:
			BlobTracker();
			virtual ~BlobTracker();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();

			virtual std::vector<FrameDrawer*> createDrawers();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			std::vector<const Blob*> getBlobs() const;

			DECLARE_OP_DESC;
		};
	}
}