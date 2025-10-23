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

#include <config.h>
#ifdef __SUPPORT_GUI

#include <vector>

#include "guiElement.h"

#include <nlohmann/json.hpp>

namespace sqid
{
	class SceneGraphDrawer;

	class OpDrawer;
	class SourceDrawer;

	class NodeDrawer : 
		public GUI::Element,
		public GUI::Hoverable,
		public GUI::Selectable,
		public GUI::Draggable
	{
	protected:
		std::vector<glm::vec2> _verts;
		std::vector<glm::vec2> _vertsSelection;

	public:
		explicit NodeDrawer( GUI::Drawer *drawer );
		virtual ~NodeDrawer();

		virtual void build()	{}

		virtual bool drawUI();

		virtual bool mouseDown( int button, int mods, bool imGuiHandled );
		virtual bool mouseUp( int button, int mods, bool imGuiHandled );

		virtual bool saveToJSON( nlohmann::json &j ) const;
		virtual bool loadFromJSON( const nlohmann::json &j );

		virtual OpDrawer *asOpDrawer()		{ return nullptr; }
		virtual SourceDrawer *asSourceDrawer()	{ return nullptr; }

		virtual bool childHit( const glm::vec2 &mousePos ) { return false; }
	};
}

#endif