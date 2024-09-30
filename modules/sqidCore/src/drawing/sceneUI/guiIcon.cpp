/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "guiIcon.h"

#ifdef __SUPPORT_GUI

#include <common.h>
#include <commonGL.h>

#include "guiDrawer.h"
#include "uiStyle.h"

namespace sqid
{
	namespace GUI
	{
		Icon::Icon( Drawer *drawer, float size ) :
			Element( drawer, GUI::Element::A_CENTER )
		{
			_size = glm::vec2( size );
		}

		Icon::~Icon()
		{}





		IconTriangle::IconTriangle( Drawer *drawer, float size ) :
			Icon( drawer, size )
		{
			_verts.resize( 3 );
			_verts[0] = glm::vec2( size / 2, 0 );
			_verts[1] = glm::vec2( size / 2 * cos( pi2() / 3 ), size / 2 * sin( pi2() / 3 ) );
			_verts[2] = glm::vec2( _verts[1].x, -_verts[1].y );

			_cols.resize( 3 );
			_cols[0] = glm::vec3( 1 );
			_cols[1] = glm::vec3( 1 );
			_cols[2] = glm::vec3( 1 );
		}

		IconTriangle::~IconTriangle()
		{}

		bool IconTriangle::draw()
		{
			if( !Icon::draw() )
				return false;

			glPushMatrix();
			glPushAttrib( GL_ALL_ATTRIB_BITS );
			{
				glm::vec2 wPos( getWorldPos() );
				glTranslatef( wPos.x, wPos.y, 0.0f );

				glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

				glColorPointer( 3, GL_FLOAT, 0, &_cols[0] );
				glDrawArrays( GL_TRIANGLES, 0, _verts.size() );
			}
			glPopAttrib();
			glPopMatrix();

			return true;
		}







		IconQuad::IconQuad( Drawer *drawer, float size ) :
			Icon( drawer, size )
		{
			_verts.resize( 4 );
			_verts[0] = glm::vec2( -size / 2, -size / 2 );
			_verts[1] = glm::vec2( size / 2, -size / 2 );
			_verts[2] = glm::vec2( size / 2, size / 2 );
			_verts[3] = glm::vec2( -size / 2, size / 2 );

			_cols.resize( 4 );
			_cols[0] = glm::vec3( 1 );
			_cols[1] = glm::vec3( 1 );
			_cols[2] = glm::vec3( 1 );
			_cols[3] = glm::vec3( 1 );
		}

		IconQuad::~IconQuad()
		{}

		bool IconQuad::draw()
		{
			if( !Icon::draw() )
				return false;

			glPushMatrix();
			glPushAttrib( GL_ALL_ATTRIB_BITS );
			{
				glm::vec2 wPos( getWorldPos() );
				glTranslatef( wPos.x, wPos.y, 0.0f );

				glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

				glColorPointer( 3, GL_FLOAT, 0, &_cols[0] );
				glDrawArrays( GL_QUADS, 0, _verts.size() );
			}
			glPopAttrib();
			glPopMatrix();

			return true;
		}




		IconCross::IconCross( Drawer *drawer, float size ) :
			Icon( drawer, size )
		{
			_verts.resize( 4 );
			_verts[0] = glm::vec2( -size / 2, -size / 2 );
			_verts[1] = glm::vec2( size / 2, size / 2 );
			_verts[2] = glm::vec2( size / 2, -size / 2 );
			_verts[3] = glm::vec2( -size / 2, size / 2 );

			_cols.resize( 4 );
			_cols[0] = glm::vec3( 1 );
			_cols[1] = glm::vec3( 1 );
			_cols[2] = glm::vec3( 1 );
			_cols[3] = glm::vec3( 1 );
		}

		IconCross::~IconCross()
		{}

		bool IconCross::draw()
		{
			if( !Icon::draw() )
				return false;

			glPushMatrix();
			glPushAttrib( GL_ALL_ATTRIB_BITS );
			{
				glLineWidth( _drawer->getUIStyle()->IconLineWidth );

				glm::vec2 wPos( getWorldPos() );
				glTranslatef( wPos.x, wPos.y, 0.0f );

				glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

				glColorPointer( 3, GL_FLOAT, 0, &_cols[0] );
				glDrawArrays( GL_LINES, 0, _verts.size() );
			}
			glPopAttrib();
			glPopMatrix();

			return true;
		}




		IconPlus::IconPlus( Drawer *drawer, float size ) :
			Icon( drawer, size )
		{
			_verts.resize( 4 );
			_verts[0] = glm::vec2( 0, -size / 2 );
			_verts[1] = glm::vec2( 0, size / 2 );
			_verts[2] = glm::vec2( -size / 2, 0 );
			_verts[3] = glm::vec2( size / 2, 0 );

			_cols.resize( 4 );
			_cols[0] = glm::vec3( 1 );
			_cols[1] = glm::vec3( 1 );
			_cols[2] = glm::vec3( 1 );
			_cols[3] = glm::vec3( 1 );
		}

		IconPlus::~IconPlus()
		{}

		bool IconPlus::draw()
		{
			if( !Icon::draw() )
				return false;

			glPushMatrix();
			glPushAttrib( GL_ALL_ATTRIB_BITS );
			{
				glLineWidth( _drawer->getUIStyle()->IconLineWidth );

				glm::vec2 wPos( getWorldPos() );
				glTranslatef( wPos.x, wPos.y, 0.0f );

				glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

				glColorPointer( 3, GL_FLOAT, 0, &_cols[0] );
				glDrawArrays( GL_LINES, 0, _verts.size() );
			}
			glPopAttrib();
			glPopMatrix();

			return true;
		}



		IconMinus::IconMinus( Drawer *drawer, float size ) :
			Icon( drawer, size )
		{
			_verts.resize( 2 );
			_verts[0] = glm::vec2( -size / 2, 0 );
			_verts[1] = glm::vec2( size / 2, 0 );

			_cols.resize( 2 );
			_cols[0] = glm::vec3( 1 );
			_cols[1] = glm::vec3( 1 );
		}

		IconMinus::~IconMinus()
		{}

		bool IconMinus::draw()
		{
			if( !Icon::draw() )
				return false;

			glPushMatrix();
			glPushAttrib( GL_ALL_ATTRIB_BITS );
			{
				glLineWidth( _drawer->getUIStyle()->IconLineWidth );

				glm::vec2 wPos( getWorldPos() );
				glTranslatef( wPos.x, wPos.y, 0.0f );

				glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

				glColorPointer( 3, GL_FLOAT, 0, &_cols[0] );
				glDrawArrays( GL_LINES, 0, _verts.size() );
			}
			glPopAttrib();
			glPopMatrix();

			return true;
		}



		IconArrowTopRight::IconArrowTopRight( Drawer *drawer, float size ) :
			Icon( drawer, size )
		{
			_verts.resize( 3 );
			_verts[0] = glm::vec2( -size / 2, -size / 2 );
			_verts[1] = glm::vec2( size / 2, size / 2 );
			_verts[2] = glm::vec2( size / 2, -size / 2 );

			_cols.resize( 3 );
			_cols[0] = glm::vec3( 1 );
			_cols[1] = glm::vec3( 1 );
			_cols[2] = glm::vec3( 1 );
		}

		IconArrowTopRight::~IconArrowTopRight()
		{}

		bool IconArrowTopRight::draw()
		{
			if( !Icon::draw() )
				return false;

			glPushMatrix();
			glPushAttrib( GL_ALL_ATTRIB_BITS );
			{
				glm::vec2 wPos( getWorldPos() );
				glTranslatef( wPos.x, wPos.y, 0.0f );

				glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

				glColorPointer( 3, GL_FLOAT, 0, &_cols[0] );
				glDrawArrays( GL_TRIANGLES, 0, _verts.size() );
			}
			glPopAttrib();
			glPopMatrix();

			return true;
		}








		IconArrowLowerLeft::IconArrowLowerLeft( Drawer *drawer, float size ) :
			Icon( drawer, size )
		{
			_verts.resize( 3 );
			_verts[0] = glm::vec2( -size / 2, -size / 2 );
			_verts[1] = glm::vec2( -size / 2, size / 2 );
			_verts[2] = glm::vec2( size / 2, size / 2 );

			_cols.resize( 3 );
			_cols[0] = glm::vec3( 1 );
			_cols[1] = glm::vec3( 1 );
			_cols[2] = glm::vec3( 1 );
		}

		IconArrowLowerLeft::~IconArrowLowerLeft()
		{}

		bool IconArrowLowerLeft::draw()
		{
			if( !Icon::draw() )
				return false;

			glPushMatrix();
			glPushAttrib( GL_ALL_ATTRIB_BITS );
			{
				glm::vec2 wPos( getWorldPos() );
				glTranslatef( wPos.x, wPos.y, 0.0f );

				glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

				glColorPointer( 3, GL_FLOAT, 0, &_cols[0] );
				glDrawArrays( GL_TRIANGLES, 0, _verts.size() );
			}
			glPopAttrib();
			glPopMatrix();

			return true;
		}
	}
}
#endif