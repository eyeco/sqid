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

#include "pinDrawer.h"
#include "guiDrawer.h"

namespace sqid
{
	class Op;
	class DataSource;
	class SceneGraph;

	class FrameBuffer;
	class NodeDrawer;
	class OpDrawer;
	class SourceDrawer;

	class SQID_API SceneGraphDrawer : public GUI::Drawer
	{
	private:
		bool _drawDebug;

		bool _contextMenuOpen;
		bool _exitContextMenu;
		bool _suppressContextMenu;

		bool _openFinder;
		bool _closeFinder;
		std::vector<char> _finderBuffer;
		int _finderSelected;

		float _maxZ;

		SceneGraph *_sg;

		OutletPinDrawer *_sticky;

		std::set<NodeDrawer*> _selectedDrawers;

		OpDrawer *_maximizedDrawer;

		FrameBuffer *_fpMaximized;
		size_t _maximizedWidth;
		size_t _maximizedHeight;

		std::vector<glm::vec2> _mapVerts;
		std::vector<glm::vec3> _mapCols;
		std::vector<glm::vec2> _mapUVs;

		std::vector<bool> _mouseDown;

		glm::vec2 _mousePos;
		glm::vec2 _creationPos;

		bool _selectionWindow;
		bool _selectionWindowAdditive;
		glm::vec2 _selectionWindowP0;

		std::map<GUID, NodeDrawer*, CompareGUID> _drawers;

		const OpDrawer *getOpDrawer( const GUID &objectID ) const;
		const SourceDrawer *getSourceDrawer( const GUID &objectID ) const;

		OpDrawer *createOpDrawer( Op *op, const glm::vec2 &pos );
		SourceDrawer *createSourceDrawer( DataSource *source, const glm::vec2 &pos );

		void select( NodeDrawer *nd, bool additive = false );
		void select( std::list<NodeDrawer*> &nds, bool additive = false );

		void deselect( NodeDrawer *nd );
		void deselect( std::list<NodeDrawer*> &nds );

		void toggleSelection( NodeDrawer *nd, bool additive = false );
		void toggleSelection( std::list<NodeDrawer*> &nds, bool additive = false );

		Rect getSelectionRect() const;

		void startDrag();

		void moveToFront( NodeDrawer *nd );

		void resortZ();

		void inletCallback( PinDrawer *pinDrawer, PinDrawer::Event e );
		void outletCallback( PinDrawer *pinDrawer, PinDrawer::Event e );

	public:
		SceneGraphDrawer( SceneGraph *sg, const UIStyle *uis );
		~SceneGraphDrawer();

		void build();

		void preDraw();
		void draw();

		void drawUI();

		bool mouseDown( int button, int mods, bool imGuiHandled );
		bool mouseUp( int button, int mods, bool imGuiHandled );
		void mouseMotion( const glm::vec2 &pos );

		bool scroll( const glm::vec2 &offset, bool imGuiHandled );

		bool keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled );
		bool charDown( unsigned char c, bool imGuiHandled );

		bool load( const std::string &scene );
		bool save( const std::string &scene );

		OpDrawer *instantiateOp( const GUID &classID, const glm::vec2 &atScreenPos );
		SourceDrawer *instantiateSource( DeviceInterface di, const glm::vec2 &atScreenPos );

		void setMaximized( OpDrawer *drawer );
		OpDrawer *getMaximized() const { return _maximizedDrawer; }

		bool getBB( Rect &bb ) const;
		bool getBBSelected( Rect &bb ) const;

		void setDrawDebug( bool ddbg ) { _drawDebug = ddbg; }
		bool getDrawDebug() const { return _drawDebug; }

		void copyToClipboard();
		void pasteFromClipboard();
	};
}
#endif