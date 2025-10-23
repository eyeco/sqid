/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "frameDrawerStatistics.h"

#ifdef __SUPPORT_GUI

#include "../processing/ops/statistics.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sqid
{
	DEFINE_FRAMEDRAWER_DESC( FrameDrawerHistogram, "histogram", "51ED100E-FEE0-49A2-98B0-B51B11046C42" );

	FrameDrawerHistogram::FrameDrawerHistogram( const Statistics::Histogram *histogram ) :
		FrameDrawer2D(),
		_histogram( histogram )
	{
	}

	FrameDrawerHistogram::~FrameDrawerHistogram()
	{
		_histogram = nullptr;
	}

	void FrameDrawerHistogram::lateDraw( const SampleFrame *sf )
	{
		FrameDrawer2D::lateDraw( sf );

		if( !_histogram )
			return;

		glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT );
		{
			glDisable( GL_TEXTURE_2D );

			auto hists = _histogram->getHistograms();

			glLineWidth( 3 );

			glEnableClientState( GL_VERTEX_ARRAY );

			glEnable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ONE );

			glm::vec3 colors[] = {
				glm::vec3( 1, 0, 0 ),
				glm::vec3( 0, 1, 0 ),
				glm::vec3( 0, 0, 1 ),
				glm::vec3( 0, 1, 1 ),
				glm::vec3( 1, 0, 1 ),
				glm::vec3( 1, 1, 0 ),
				glm::vec3( 1, 1, 1 )
			};

			int cntr = 0;
			for( auto &it : hists )
			{
				if( it.size().width != 1 || it.channels() != 1 )
				{
					std::cerr << "<warning> unexpected histogram format (" << it.size().width << "x" << it.size().height << "x" << it.channels() << ")" << std::endl;
					continue;
				}

				bool autoScale = false;

				float scale = 1.0f;
				int bins = it.size().height;

				if( !bins )
				{
					std::cerr << "<warning> no bins in histogram" << std::endl;
					continue;
				}

				if( autoScale )
				{
					float maxValue = it.at<float>( 0 );
					for( int i = 1; i < bins; i++ )
						if( it.at<float>( i ) > maxValue )
							maxValue = it.at<float>( i );
					scale = 1.0f / maxValue;
				}

				std::vector<GLfloat> vertices;

				vertices.resize( bins * 4 );
				for( int i = 0; i < bins; i++ )
				{
					vertices[i * 4 + 0] = ( (float) i / ( bins - 1.0f ) ) * 2.0f - 1.0f;
					vertices[i * 4 + 1] = -1;
					vertices[i * 4 + 2] = vertices[i * 4 + 0];
					vertices[i * 4 + 3] = ( it.at<float>( i ) * scale * 2.0f - 1.0f );
				}

				glColor3fv( glm::value_ptr( colors[cntr % arraySize( colors )] * 0.5f ) );

				glVertexPointer( 2, GL_FLOAT, 0, &vertices[0] );
				glDrawArrays( GL_QUAD_STRIP, 0, (GLsizei) ( vertices.size() / 2 ) );

				vertices.resize( ( bins + 2 ) * 2 );
				for( int i = 0; i < bins; i++ )
				{
					vertices[i * 2 + 0] = ( (float) i / ( bins - 1.0f ) ) * 2.0f - 1.0f;
					vertices[i * 2 + 1] = ( it.at<float>( i ) * scale * 2.0f - 1.0f );
				}

				vertices[bins * 2 + 0] = 1.0f;
				vertices[bins * 2 + 1] = -1.0f;
				vertices[bins * 2 + 2] = -1.0f;
				vertices[bins * 2 + 3] = -1.0f;

				glColor3fv( glm::value_ptr( colors[cntr % arraySize( colors )] ) );

				glVertexPointer( 2, GL_FLOAT, 0, &vertices[0] );
				glDrawArrays( GL_LINE_LOOP, 0, (GLsizei) ( vertices.size() / 2 ) );

				cntr++;
			}

			// deactivate vertex arrays after drawing
			glDisableClientState( GL_VERTEX_ARRAY );
		}
		glPopAttrib();
	}
}
#endif