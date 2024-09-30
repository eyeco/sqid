/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "nodeDrawer.h"

#ifdef __SUPPORT_GUI

//#include <app.h>
#include <fileIO/json.h>

#include "guiDrawer.h"
#include "uiStyle.h"


namespace sqid
{
	NodeDrawer::NodeDrawer( GUI::Drawer *drawer ) :
		GUI::Element( drawer ),
		GUI::Hoverable(),
		GUI::Selectable(),
		GUI::Draggable()
	{
		_size = glm::vec2(
			_drawer->getUIStyle()->SourceNodeWidth,
			_drawer->getUIStyle()->SourceNodeHeight );
	}

	NodeDrawer::~NodeDrawer()
	{}

	bool NodeDrawer::drawUI()
	{
		bool ret = GUI::Element::drawUI();

		if( getSelected() )
			ret |= true;

		return ret;
	}

	bool NodeDrawer::mouseDown( int button, int mods, bool imGuiHandled )
	{
		bool ret = GUI::Element::mouseDown( button, mods, imGuiHandled );

		if( !imGuiHandled && button == 0 )
		{
			if( getHovered() )
				ret = true;
		}

		return ret;
	}

	bool NodeDrawer::mouseUp( int button, int mods, bool imGuiHandled )
	{
		bool ret = GUI::Element::mouseUp( button, mods, imGuiHandled );

		if( !imGuiHandled && button == 0 )
		{
			setDragging( false );

			if( getHovered() )
				ret = true;
		}

		return ret;
	}

	bool NodeDrawer::saveToJSON( nlohmann::json &j ) const
	{
		save( j, "x", _pos.x );
		save( j, "y", _pos.y );

		save( j, "z", _z );

		return true;
	}

	bool NodeDrawer::loadFromJSON( const nlohmann::json &j )
	{
		glm::vec2 p;
		if( load<float>( j, "x", p.x ) && load<float>( j, "y", p.y ) )
			setPos( p );

		load<float>( j, "z", _z );

		return true;
	}
}


#endif