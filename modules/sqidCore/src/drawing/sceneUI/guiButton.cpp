/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "guiButton.h"

#ifdef __SUPPORT_GUI

#include <common.h>
#include <commonGL.h>

#include "guiIcon.h"
#include "uiStyle.h"
#include "guiDrawer.h"

namespace sqid
{
	namespace GUI
	{
		namespace Internal
		{
			std::vector<glm::vec2> buildButtonVerts( float width )
			{
				std::vector<glm::vec2> buttonVerts( 4 );

				buttonVerts[0] = glm::vec2( -width / 2, -width / 2 );
				buttonVerts[1] = glm::vec2( -width / 2, width / 2 );
				buttonVerts[2] = glm::vec2( width / 2, width / 2 );
				buttonVerts[3] = glm::vec2( width / 2, -width / 2 );

				return buttonVerts;
			}

			std::vector<glm::vec3> buildButtonColors( const Drawer* d )
			{
				std::vector<glm::vec3> cols;

				cols.assign( 4, d->getUIStyle()->ButtonColorBG );

				return cols;
			}

			std::vector<glm::vec3> buildButtonColorsHovered( const glm::vec3 &c )
			{
				std::vector<glm::vec3> cols;

				cols.assign( 4, c );

				return cols;
			}

			std::vector<glm::vec3> buildButtonColorsActive( const glm::vec3 &c )
			{
				std::vector<glm::vec3> cols;

				cols.assign( 4, c );

				return cols;
			}

			std::vector<glm::vec3> buildButtonColorsActiveHovered( const glm::vec3 &c )
			{
				std::vector<glm::vec3> cols;

				cols.assign( 4, c );

				return cols;
			}

			std::vector<glm::vec3> buildButtonColorsPressed( const glm::vec3 &c )
			{
				std::vector<glm::vec3> cols;

				cols.assign( 4, c );

				return cols;
			}

			const std::vector<glm::vec2> &getButtonVerts( const Drawer* d )
			{
				static const std::vector<glm::vec2> pinVerts = Internal::buildButtonVerts( d->getUIStyle()->NodePinWidth );
				return pinVerts;
			}

			const std::vector<glm::vec3> &getButtonColors( const Drawer* d )
			{
				static const std::vector<glm::vec3> cols = Internal::buildButtonColors( d );
				return cols;
			}

			const std::vector<glm::vec3> &getButtonColorsHovered( const Drawer* d )
			{
				static const std::vector<glm::vec3> cols = Internal::buildButtonColorsHovered( d->getUIStyle()->ButtonColorBG * 1.2f );
				return cols;
			}

			const std::vector<glm::vec3> &getButtonColorsActive( const Drawer* d )
			{
				static const std::vector<glm::vec3> cols = Internal::buildButtonColorsActive( d->getUIStyle()->ButtonColorActiveBG );
				return cols;
			}

			const std::vector<glm::vec3> &getButtonColorsActiveHovered( const Drawer* d )
			{
				static const std::vector<glm::vec3> cols = Internal::buildButtonColorsActiveHovered( d->getUIStyle()->ButtonColorActiveBG * 1.2f );
				return cols;
			}

			const std::vector<glm::vec3> &getButtonColorsPressed( const Drawer* d )
			{
				static const std::vector<glm::vec3> cols = Internal::buildButtonColorsPressed( d->getUIStyle()->ButtonColorPressedBG );
				return cols;
			}


		}

		Button::Button( Drawer *drawer, bool toggle, Icon *icon, Icon *iconActive ) :
			Element( drawer, GUI::Element::A_CENTER ),
			_toggle( toggle ),
			_active( false ),
			_icon( icon ),
			_iconActive( iconActive )
		{
			_size = glm::vec2( 
				_drawer->getUIStyle()->NodeButtonWidth,
				_drawer->getUIStyle()->NodeButtonWidth
			);

			if( _icon )
			{
				_icon->setParent( this );
				_icon->setPos( glm::vec2() );
			}
			if( _iconActive )
			{
				_iconActive->setParent( this );
				_iconActive->setPos( glm::vec2() );
			}

			setActive( _active );
		}

		Button::~Button()
		{
			safeDelete( _icon );
			safeDelete( _iconActive );
		}

		bool Button::draw()
		{
			if( !Element::draw() )
				return false;

			auto cols =
				( getPressed() ? Internal::getButtonColorsPressed( _drawer ) :
				( getActive() ? ( getHovered() ? Internal::getButtonColorsActiveHovered( _drawer ) : Internal::getButtonColorsActive( _drawer ) ) :
				( getHovered() ? Internal::getButtonColorsHovered( _drawer ) : Internal::getButtonColors( _drawer ) ) ) );

			glPushMatrix();
			glPushAttrib( GL_ALL_ATTRIB_BITS );
			{
				glm::vec2 wPos( getWorldPos() );
				glTranslatef( wPos.x, wPos.y, 0.0f );

				glVertexPointer( 2, GL_FLOAT, 0, &Internal::getButtonVerts( _drawer )[0] );

				glColorPointer( 3, GL_FLOAT, 0, &cols[0] );
				glDrawArrays( GL_QUADS, 0, cols.size() );
			}
			glPopAttrib();
			glPopMatrix();

			return true;
		}

		bool Button::mouseDown( int button, int mods, bool imGuiHandled )
		{
			bool ret = GUI::Element::mouseDown( button, mods, imGuiHandled );

			if( !imGuiHandled && button == 0 )
			{
				if( getHovered() )
				{
					setPressed( true );

					if( _toggle )
						setActive( !getActive() );

					if( _cb )
						_cb( this, Button::E_PRESSED );
				}
			}

			return ret;
		}

		bool Button::mouseUp( int button, int mods, bool imGuiHandled )
		{
			bool ret = GUI::Element::mouseUp( button, mods, imGuiHandled );

			if( button == 0 && getPressed() )
			{
				setPressed( false );
				if( _cb )
					_cb( this, Button::E_RELEASED );
			}

			return ret;
		}

		float Button::setZ( float z )
		{
			z = Element::setZ( z );

			if( _icon )
				z = _icon->setZ( z + 0.001f );
			if( _iconActive )
				z = _iconActive->setZ( z + 0.001f );

			return z;
		}

		void Button::setActive( bool active )
		{ 
			_active = active;

			if( _icon )
				_icon->setVisible( !active );
			if( _iconActive )
				_iconActive->setVisible( active );
		}
	}
}

#endif