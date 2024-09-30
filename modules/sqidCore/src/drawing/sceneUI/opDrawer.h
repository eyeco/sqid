/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once 

#include <config.h>
#ifdef __SUPPORT_GUI

#include <guid.h>

#include "nodeDrawer.h"
#include "pinDrawer.h"
#include "guiButton.h"

namespace sqid
{
	class Op;
	class DataSource;
	class FrameDrawer;
	class SampleFrame;

	class OpDrawer : public NodeDrawer
	{
	private:
		Op *_op;

		std::vector<FrameDrawer*> _drawers;
		FrameDrawer *_currentDrawer;

		std::vector<glm::vec2> _mapVerts;
		std::vector<glm::vec2> _mapUVs;

		bool _collapsed;
		bool _maximized;

		std::vector<InletPinDrawer*> _inlets;
		std::vector<OutletPinDrawer*> _outlets;

		std::map<std::string, InletPinDrawer*> _inletsMap;
		std::map<std::string, OutletPinDrawer*> _outletsMap;

		GUI::Button *_collapseButton;
		GUI::Button *_maximizeButton;

		std::vector<GUI::Button*> _buttons;

		FrameDrawer *getDrawer( const GUID &did ) const;

		void rebuild();

		void collapseButtonCallback( const GUI::Button *button, GUI::Button::Event e );
		void maximizeButtonCallback( const GUI::Button *button, GUI::Button::Event e );

	public:
		OpDrawer( GUI::Drawer *drawer, Op *op, const glm::vec2 &pos, PinDrawer::PinDrawerCallback inletCallback, PinDrawer::PinDrawerCallback outletCallback );
		virtual ~OpDrawer();

		virtual void build();

		virtual void preDraw();
		virtual bool draw();

		virtual bool drawUI();

		virtual bool mouseDown( int button, int mods, bool imGuiHandled );
		virtual bool mouseUp( int button, int mods, bool imGuiHandled );

		virtual float setZ( float z );

		virtual bool saveToJSON( nlohmann::json &j ) const;
		virtual bool loadFromJSON( const nlohmann::json &j );

		bool update( const SampleFrame *sf ) const;

		bool getMaximized() const { return _maximized; }
		void setMaximized( bool maximized );

		Op *getOp() const { return _op; }

		virtual OpDrawer *asOpDrawer() { return this; }

		const InletPinDrawer *getInlet( const std::string &name ) const;
		const OutletPinDrawer *getOutlet( const std::string &name ) const;

		const std::vector<InletPinDrawer*> &getInlets() const	{ return _inlets; }
		const std::vector<OutletPinDrawer*> &getOutlets() const	{ return _outlets; }

		FrameDrawer *getCurrentDrawer() const { return _currentDrawer; }

		virtual bool childHit( const glm::vec2 &mousePos );
	};
}
#endif