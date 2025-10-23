/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "pinDrawer.h"

#ifdef __SUPPORT_GUI

#include <commonGL.h>
#include <commonImGui.h>

#include "guiDrawer.h"
#include "uiStyle.h"

namespace sqid
{
	namespace Internal
	{
		std::vector<glm::vec2> buildPinVerts( float w )
		{
			std::vector<glm::vec2> pinVerts( 4 );

			pinVerts[0] = glm::vec2( -w * 0.5f, w * 0.5f );
			pinVerts[1] = glm::vec2( w * 0.5f, w * 0.5f );
			pinVerts[2] = glm::vec2( w * 0.5f, -w * 0.5f );
			pinVerts[3] = glm::vec2( -w * 0.5f, -w * 0.5f );

			return pinVerts;
		}

		std::vector<glm::vec3> buildPinColors( const glm::vec3 &col )
		{
			std::vector<glm::vec3> pinColors( 4 );

			for( auto &it : pinColors )
				it = col;

			return pinColors;
		}

		const std::vector<glm::vec2> &getPinVerts( const GUI::Drawer* d )
		{
			static const std::vector<glm::vec2> pinVerts = Internal::buildPinVerts( d->getUIStyle()->NodePinWidth );
			return pinVerts;
		}

		const std::vector<glm::vec3> &getPinColors( const GUI::Drawer* d )
		{
			static const std::vector<glm::vec3> cols = Internal::buildPinColors( d->getUIStyle()->PinColor );
			return cols;
		}

		const std::vector<glm::vec3> &getPinColorsHovered( const GUI::Drawer* d )
		{
			static const std::vector<glm::vec3> cols = Internal::buildPinColors( d->getUIStyle()->PinColorHovered );
			return cols;
		}

		const std::vector<glm::vec3> &getPinColorsActivity( const GUI::Drawer* d )
		{
			static const std::vector<glm::vec3> cols = Internal::buildPinColors( d->getUIStyle()->PinColorActivity );
			return cols;
		}

		const std::vector<glm::vec3> &getPinColorsCompatible( const GUI::Drawer* d )
		{
			static const std::vector<glm::vec3> cols = Internal::buildPinColors( d->getUIStyle()->PinColorCompatible );
			return cols;
		}
	}

	PinDrawer::PinDrawer( GUI::Drawer *drawer ) :
		GUI::Element( drawer, GUI::Element::A_CENTER ),
		GUI::Hoverable(),
		_drawCompatible( false )
	{
		_size = glm::vec2( 
			_drawer->getUIStyle()->NodePinWidth,
			_drawer->getUIStyle()->NodePinWidth );
	}

	PinDrawer::~PinDrawer()
	{}

	bool PinDrawer::draw()
	{
		if( !GUI::Element::draw() )
			return false;

		glPushMatrix();
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glm::vec2 wPos( getWorldPos() );
			glTranslatef( wPos.x, wPos.y, 0.0f );

			glVertexPointer( 2, GL_FLOAT, 0, &Internal::getPinVerts( _drawer )[0] );

			if( getHovered() )
				glColorPointer( 3, GL_FLOAT, 0, &Internal::getPinColorsHovered( _drawer )[0] );
			else if( _drawCompatible )
				glColorPointer( 3, GL_FLOAT, 0, &Internal::getPinColorsCompatible( _drawer )[0] );
			else if( getPin()->activity() )
				glColorPointer( 3, GL_FLOAT, 0, &Internal::getPinColorsActivity( _drawer )[0] );
			else
				glColorPointer( 3, GL_FLOAT, 0, &Internal::getPinColors( _drawer )[0] );
			glDrawArrays( GL_QUADS, 0, Internal::getPinVerts( _drawer ).size() );
		}
		glPopAttrib();
		glPopMatrix();

		return true;
	}

	bool PinDrawer::drawUI()
	{
		bool ret = GUI::Element::drawUI();

		if( getHovered() )
		{
			glm::vec2 wPos( getWorldPos() );
			ImGui::SetNextWindowPos( glm2im( ( _drawer->getMV() * ( glm::vec4( wPos.x - 5, wPos.y - 35, 0, 1 ) ) ).xy ) );

			auto pin = getPin();
			ImGui::SetTooltip( "%s [%d x %s]:\n%s", pin->getName().c_str(), pin->getDataCntr(), pin->getTypeShorteName().c_str(), pin->getContentDesc().c_str() );
		}

		return ret;
	}

	bool PinDrawer::mouseDown( int button, int mods, bool imGuiHandled )
	{
		bool ret = GUI::Element::mouseDown( button, mods, imGuiHandled );

		if( imGuiHandled )
			return ret;

		if( button == 0 && getHovered() )
		{
			if( _cb )
				_cb( this, PinDrawer::E_PRESSED );

			ret = true;
		}

		return ret;
	}

	bool PinDrawer::mouseUp( int button, int mods, bool imGuiHandled )
	{
		bool ret = GUI::Element::mouseUp( button, mods, imGuiHandled );

		if( imGuiHandled )
			return ret;

		if( button == 0 && getHovered() )
		{
			if( _cb )
				_cb( this, PinDrawer::E_RELEASED );

			ret = true;
		}

		return ret;
	}

	void PinDrawer::updateCompatibility( const PinDrawer *other )
	{
		if( !other || !other->getPin() )
			_drawCompatible = false;
		else
			_drawCompatible = getPin()->isCompatible( other->getPin() );
	}





	InletPinDrawer::InletPinDrawer( GUI::Drawer *drawer, InletPin *pin ) :
		PinDrawer( drawer ),
		_pin( pin )
	{}


	OutletPinDrawer::OutletPinDrawer( GUI::Drawer *drawer, OutletPin *pin ) :
		PinDrawer( drawer ),
		_pin( pin )
	{}
}
#endif