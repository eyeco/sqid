/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "frameDrawerTUIO2.h"


#ifdef __SUPPORT_GUI

#include "TUIO2Ops.h"

#include <sampleFrame.h>

#include <drawing/frameBuffer.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sqid
{
	DEFINE_FRAMEDRAWER_DESC( FrameDrawerTUIO2, "TUIO", "6BA16A9D-3CC4-4636-AB06-9C0CA81EF553" );

	FrameDrawerTUIO2::FrameDrawerTUIO2( const Plugins::TUIO *tuio ) :
		FrameDrawer( 512 ),
		_tuio( tuio )
	{
	}

	FrameDrawerTUIO2::~FrameDrawerTUIO2()
	{
		_tuio = nullptr;
	}

	bool FrameDrawerTUIO2::update( const SampleFrame *sf )
	{
		//NOTE: sf is not supposed to hold data
		//TODO: implement
		return true;
	}

	void FrameDrawerTUIO2::draw( FrameBuffer *fb, size_t width, size_t height )
	{
		if( fb )
			fb->activate();

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glViewport( 0, 0, width, height );

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			glClearColor( 1, 0, 1, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			lateDraw( _sf );
		}
		glPopAttrib();

		if( fb )
			fb->deactivate();
	}

	void FrameDrawerTUIO2::lateDraw( const SampleFrame *sf )
	{
		if( !_tuio )
			return;

		auto pointers = _tuio->getPointers();

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glClearColor( 1, 1, 1, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			glEnable( GL_COLOR_MATERIAL );
			glDisable( GL_TEXTURE_2D );

			glLineWidth( 2.0f );

			float crossSize = 16.0f / getRTSize();
			float textScale = getRTSize() / 256.0f;
			float textDist = getRTSize() / 16.0f;

			for( auto &p : pointers )
			{
				glColor3f( 1, 0, 0 );

				float cx = ( p.pos.x - 0.5f ) * 2.0f;
				float cy = -( p.pos.y - 0.5f ) * 2.0f;

				glBegin( GL_LINES );
				{
					glVertex2f( cx - crossSize, cy - crossSize );
					glVertex2f( cx + crossSize, cy + crossSize );

					glVertex2f( cx - crossSize, cy + crossSize );
					glVertex2f( cx + crossSize, cy - crossSize );
				}
				glEnd();

				glColor3f( 0, 0, 1 );
				glBegin( GL_LINE_STRIP );
				{
					for( auto &v : p.history )
					{
						float hx = ( v.x - 0.5f ) * 2.0f;
						float hy = -( v.y - 0.5f ) * 2.0f;

						glVertex2f( hx, hy );
					}
				}
				glEnd();

				char tempStr[128];
				sprintf( tempStr, "id:  %d", p.id );
				printText( tempStr,
					p.pos.x * getRTSize(), p.pos.y * getRTSize(),
					getRTSize(), getRTSize(),
					glm::vec4( 0, 0, 0, 1 ), textScale );
				sprintf( tempStr, "his: %zd", p.history.size() );
				printText( tempStr,
					p.pos.x * getRTSize(), p.pos.y * getRTSize() + textDist,
					getRTSize(), getRTSize(),
					glm::vec4( 0, 0, 0, 1 ), textScale );
			}
		}
		glPopAttrib();
	}
}
#endif