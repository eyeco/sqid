/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "statistics.h"

#include "../../drawing/frameDrawerStatistics.h"

#include <fileIO/json.h>

#include <imgui/imgui.h>

namespace sqid
{
	namespace Statistics
	{
		DEFINE_OP_DESC( CenterOfMass, "centerOfMass", "/math/statistics",
			"F0A5E801-618B-459F-B1FD-5FEA092819FF" );
		DEFINE_OP_DESC( Histogram, "histogram", "/math/statistics",
			"57170399-2EB3-43FB-8BF1-F93D42D99727" );




		CenterOfMass::CenterOfMass() :
			Op()
		{
		}

		CenterOfMass::~CenterOfMass()
		{}

		bool CenterOfMass::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				int dims = 0;

				if( sf->depth() > 1 )
					dims++;
				if( sf->width() > 1 )
					dims++;
				if( sf->height() > 1 )
					dims++;
				if( !dims )
					dims = 1;

				const float *src = sf->values();
				float com[3];
				float sum = 0;

				com[0] = 0;
				com[1] = 0;
				com[2] = 0;

				for( int j = 0; j < sf->height(); j++ )
					for( int i = 0; i < sf->width(); i++ )
						for( int k = 0; k < sf->depth(); k++ )
						{
							com[0] += *src * i;
							com[1] += *src * j;
							com[2] += *src * k;

							sum += *src;

							src++;
						}

				if( std::abs( sum ) > std::numeric_limits<float>::epsilon() )
				{
					float s = 1.0f / sum;

					//scale to range [0 1]
					if( sf->width() > 1 )
						com[0] *= s / ( sf->width() - 1 );
					if( sf->height() > 1 )
						com[1] *= s / ( sf->height() - 1 );
					if( sf->depth() > 1 )
						com[2] *= s / ( sf->depth() - 1 );

					SampleFrame *ret = new SampleFrame( dims, 1, sf->timeStamp(), 1 );
					for( int i = 0; i < dims; i++ )
						ret->values()[i] = com[i];

					drawFrame( ret );

					pushOutput( "out", ret );
					safeDelete( ret );
				}

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool CenterOfMass::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			return ret;
		}

		bool CenterOfMass::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			return ret;
		}




		Histogram::Histogram( size_t bins ) :
			Op(),
			_bins( bins )
		{
		}

		Histogram::~Histogram()
		{}

#ifdef __SUPPORT_GUI
		bool Histogram::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int i = _bins;
			if( ImGui::SliderInt( "min", &i, 1, 100 ) )
				_bins = i;

			return true;
		}

		std::vector<FrameDrawer*> Histogram::createDrawers()
		{
			std::vector<FrameDrawer*> drawers;

			drawers.push_back( new FrameDrawerHistogram( this ) );

			return drawers;
		}
#endif

		bool Histogram::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				std::vector<cv::Mat> planes;
				cv::split( sf->mat(), planes );

				_histograms.resize( planes.size() );

				if( planes.size() )
				{
					bool uniform = true;
					bool accumulate = false;

					int histSize = _bins;

					float range[] = { 0, 1.0f }; //NOTE: the upper bound is exclusive
					const float *histRange = { range };

					for( int i = 0; i < planes.size(); i++ )
					{
						cv::calcHist( &planes[i], 1, 0, cv::Mat(), _histograms[i], 1, &histSize, &histRange, uniform, accumulate );
						cv::normalize( _histograms[i], _histograms[i] );
					}

					SampleFrame *ret = new SampleFrame( histSize, planes.size(), sf->timeStamp() );
					float *dest = ret->values();
					for( int j = 0; j < planes.size(); j++ )
						for( int i = 0; i < histSize; i++ )
							*( dest++ ) = _histograms[j].at<float>( i );
					pushOutput( "out", ret );
					safeDelete( ret );
				}

				//TODO: adjust framedrawer -- it's not common to use the input frame
				// for drawing, but this is the way this particular drawer is built...
				drawFrame( sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Histogram::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<size_t>( j, "bins", _bins );

			return ret;
		}

		bool Histogram::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "bins", _bins );

			return ret;
		}
	}
}