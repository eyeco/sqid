/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "frameDrawerTracking.h"

#ifdef __SUPPORT_GUI

#include "../processing/ops/tracking.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sqid
{
	DEFINE_FRAMEDRAWER_DESC( FrameDrawerBlob, "blobs", "54BA9D43-7663-4EE7-9C3D-28005F233D79" );

	FrameDrawerBlob::FrameDrawerBlob( const Tracking::BlobTracker *tracker ) :
		FrameDrawer2D(),
		_tracker( tracker )
	{
	}

	FrameDrawerBlob::~FrameDrawerBlob()
	{
		_tracker = nullptr;
	}

	void FrameDrawerBlob::lateDraw( const SampleFrame *sf )
	{
		FrameDrawer2D::lateDraw( sf );

		if( !_tracker )
			return;

		if( !sf )
			return;

		auto blobs = _tracker->getBlobs();

		float w = sf->width();
		float h = sf->height();
		float pxSizeX = 2.0f / w;
		float pxSizeY = 2.0f / h;

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glEnable( GL_COLOR_MATERIAL );
			glDisable( GL_TEXTURE_2D );

			glLineWidth( 2.0f );

			for( auto &b : blobs )
			{
				float cx = ( b->pos.x + 0.5f ) * pxSizeX - 1;
				float cy = -( ( b->pos.y + 0.5f ) * pxSizeY - 1 );
				float crossSize = 0.05f;

				glColor3f( 1, 1, 0 );
				glBegin( GL_LINE_STRIP );
				{
					for( auto &v : b->history )
						glVertex2f(
						( v.x + 0.5f ) * pxSizeX - 1,
							-( ( v.y + 0.5f ) * pxSizeY - 1 ) );
				}
				glEnd();

				glColor3f( 0, 1, 1 );
				glBegin( GL_LINES );
				{
					glVertex2f( cx - crossSize, cy - crossSize );
					glVertex2f( cx + crossSize, cy + crossSize );

					glVertex2f( cx - crossSize, cy + crossSize );
					glVertex2f( cx + crossSize, cy - crossSize );
				}
				glEnd();

				int circleResolution = 25;
				glBegin( GL_LINE_LOOP );
				{
					for( int i = 0; i < circleResolution; i++ )
					{
						glVertex2f(
							cx + cos( pi2() * i / circleResolution ) * b->size * pxSizeX * 0.5f,
							cy + sin( pi2() * i / circleResolution ) * b->size * pxSizeY * 0.5f );
					}
				}
				glEnd();

				if( b->angle > 0.0f )
				{
					glColor3f( 0, 0, 1 );

					glBegin( GL_LINES );
					{
						glVertex2f( cx, cy );
						glVertex2f(
							cx + cos( pi2() * b->angle / 360.0f ) * b->size * pxSizeX * 0.55f,
							cy + sin( pi2() * b->angle / 360.0f ) * b->size * pxSizeY * 0.55f );
					}
					glEnd();
				}

				char tempStr[128];
				sprintf( tempStr, "id:  %d", b->id );
				printText( tempStr,
					b->pos.x / getWidth() * getRTSize(), b->pos.y / getHeight() * getRTSize(),
					getRTSize(), getRTSize(),
					glm::vec4( 1 ), 3.0f );
				sprintf( tempStr, "his: %zd", b->history.size() );
				printText( tempStr,
					b->pos.x / getWidth() * getRTSize(), b->pos.y / getHeight() * getRTSize() + getRTSize() / 16.0f,
					getRTSize(), getRTSize(),
					glm::vec4( 1 ), 3.0f );
			}
		}
		glPopAttrib();
	}
}
#endif