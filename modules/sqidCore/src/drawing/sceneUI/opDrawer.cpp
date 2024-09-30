/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "opDrawer.h"

#ifdef __SUPPORT_GUI

#include <commonImGui.h>

#include <fileIO/json.h>

#include <processing/op.h>

#include "../font.h"
#include <drawing/frameDrawer.h>
#include "guiIcon.h"
#include "guiDrawer.h"
#include "uiStyle.h"
#include "sceneGraphDrawer.h"

namespace sqid
{
	OpDrawer::OpDrawer( GUI::Drawer *drawer, Op *op, const glm::vec2 &pos, PinDrawer::PinDrawerCallback inletCallback, PinDrawer::PinDrawerCallback outletCallback ) :
		NodeDrawer( drawer ),
		_op( op ),
		_currentDrawer( nullptr ),
		_collapsed( false ),
		_maximized( false ),
		_collapseButton( nullptr ),
		_maximizeButton( nullptr )
	{
		_pos = pos;

		int maxPins = max( _op->getInlets().size(), _op->getOutlets().size() );
		_size = glm::vec2(
			max( _drawer->getUIStyle()->NodeMinWidth, _drawer->getUIStyle()->PinPadding * 2 + max( maxPins - 1, 0 ) * _drawer->getUIStyle()->NodePinDist ),
			_drawer->getUIStyle()->NodeMapDY + _drawer->getUIStyle()->NodeMapWidth + _drawer->getUIStyle()->NodePaddingY );

		for( auto &it : _op->getInlets() )
		{
			InletPinDrawer *ipd = new InletPinDrawer( drawer, it.second );
			ipd->setParent( this );
			ipd->setCallback( inletCallback );
			_inlets.push_back( ipd );
			_inletsMap.insert( std::make_pair( it.second->getName(), ipd ) );
		}

		for( auto &it : _op->getOutlets() )
		{
			OutletPinDrawer *opd = new OutletPinDrawer( drawer, it.second );
			opd->setParent( this );
			opd->setCallback( outletCallback );
			_outlets.push_back( opd );
			_outletsMap.insert( std::make_pair( it.second->getName(), opd ) );
		}

		_collapseButton = new GUI::Button( drawer, true, 
			new GUI::IconMinus( drawer, _drawer->getUIStyle()->NodeButtonWidth / 2 ),
			new GUI::IconPlus( drawer, _drawer->getUIStyle()->NodeButtonWidth / 2 ) );
		_collapseButton->setParent( this );
		_collapseButton->setPos( glm::vec2( 0, _drawer->getUIStyle()->NodeMinHeight / 2 ) );
		_collapseButton->setCallback( 
			[this] ( const GUI::Button *b, GUI::Button::Event e )
			{
				this->collapseButtonCallback( b, e );
			} );

		_maximizeButton = new GUI::Button( drawer, true, 
			new GUI::IconArrowTopRight( drawer, _drawer->getUIStyle()->NodeButtonWidth / 2 ),
			new GUI::IconArrowLowerLeft( drawer, _drawer->getUIStyle()->NodeButtonWidth / 2 ) );
		_maximizeButton->setParent( this );
		_maximizeButton->setPos( glm::vec2( _size.x, 0 ) );
		_maximizeButton->setCallback( 
			[this] ( const GUI::Button *b, GUI::Button::Event e )
			{
				this->maximizeButtonCallback( b, e );
			} );

		_buttons.push_back( _collapseButton );
		_buttons.push_back( _maximizeButton );


		if( _op )
		{
			_op->setDrawer( this );
			_drawers = _op->createDrawers();
			if( _drawers.size() )
				_currentDrawer = _drawers.front();
		}
	}

	OpDrawer::~OpDrawer()
	{
		_op = nullptr;

		_currentDrawer = nullptr;
		for( auto &it : _drawers )
			safeDelete( it );
		_drawers.clear();

		for( auto &it : _inlets )
			safeDelete( it );
		_inlets.clear();

		for( auto &it : _outlets )
			safeDelete( it );
		_outlets.clear();

		_collapseButton = nullptr;
		_maximizeButton = nullptr;
		for( auto &it : _buttons )
			safeDelete( it );
		_buttons.clear();

		_inletsMap.clear();
		_outletsMap.clear();
	}

	void OpDrawer::build()
	{
		_mapVerts.resize( 4 );
		_mapVerts[0] = glm::vec2( _drawer->getUIStyle()->NodePaddingX, _drawer->getUIStyle()->NodeMapDY );
		_mapVerts[1] = glm::vec2( _drawer->getUIStyle()->NodePaddingX, _drawer->getUIStyle()->NodeMapDY + _drawer->getUIStyle()->NodeMapWidth );
		_mapVerts[2] = glm::vec2( _drawer->getUIStyle()->NodePaddingX + _drawer->getUIStyle()->NodeMapWidth, _drawer->getUIStyle()->NodeMapDY + _drawer->getUIStyle()->NodeMapWidth );
		_mapVerts[3] = glm::vec2( _drawer->getUIStyle()->NodePaddingX + _drawer->getUIStyle()->NodeMapWidth, _drawer->getUIStyle()->NodeMapDY );

		_mapUVs.resize( 4 );
		_mapUVs[0] = glm::vec2( 0, 1 );
		_mapUVs[1] = glm::vec2( 0, 0 );
		_mapUVs[2] = glm::vec2( 1, 0 );
		_mapUVs[3] = glm::vec2( 1, 1 );

		rebuild();
	}

	void OpDrawer::rebuild()
	{
		int maxPins = max( _op->getInlets().size(), _op->getOutlets().size() );
		glm::vec2 size = glm::vec2(
			max( _drawer->getUIStyle()->NodeMinWidth, _drawer->getUIStyle()->PinPadding * 2 + max( maxPins - 1, 0 ) * _drawer->getUIStyle()->NodePinDist ),
			_drawer->getUIStyle()->NodeMapDY + ( _collapsed ? 0 : _drawer->getUIStyle()->NodeMapWidth ) + _drawer->getUIStyle()->NodePaddingY );

		for( int i = 0; i < _inlets.size(); i++ )
			_inlets[i]->setPos( glm::vec2( _drawer->getUIStyle()->PinPadding + i * _drawer->getUIStyle()->NodePinDist, 0 ) );
		for( int i = 0; i < _outlets.size(); i++ )
			_outlets[i]->setPos( glm::vec2( _drawer->getUIStyle()->PinPadding + i * _drawer->getUIStyle()->NodePinDist, size.y ) );

		_size = size;

		_verts.resize( 4 );
		_verts[0] = glm::vec2( 0, 0 );
		_verts[1] = glm::vec2( 0, _size.y );
		_verts[2] = glm::vec2( _size.x, _size.y );
		_verts[3] = glm::vec2( _size.x, 0 );

		_vertsSelection.resize( 4 );
		_vertsSelection[0] = _verts[0] + glm::vec2( -5, -5 );
		_vertsSelection[1] = _verts[1] + glm::vec2( -5, 5 );
		_vertsSelection[2] = _verts[2] + glm::vec2( 5, 5 );
		_vertsSelection[3] = _verts[3] + glm::vec2( 5, -5 );
	}
	
	void OpDrawer::preDraw()
	{
		NodeDrawer::preDraw();

		if( _currentDrawer && !_collapsed && !_clipped )
			_currentDrawer->draw();
	}

	bool OpDrawer::draw()
	{
		if( !NodeDrawer::draw() )
			return false;

		glPushMatrix();
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glm::vec2 wPos( getWorldPos() );
			glTranslatef( wPos.x, wPos.y, 0.0f );

			glm::vec3 cols[4];

			if( getSelected() )
			{
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeSelectionLineColor;

				glLineWidth( _drawer->getUIStyle()->NodeSelectionLineWidth );

				glColorPointer( 3, GL_FLOAT, 0, cols );
				glVertexPointer( 2, GL_FLOAT, 0, &_vertsSelection[0] );
				glDrawArrays( GL_LINE_LOOP, 0, _vertsSelection.size() );
			}

			if( getDragging() )
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeColorDraggingBG;
			else if( getHovered() )
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeColorHoveredBG;
			else if( !_op->getEnabled() )
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeColorDisabledBG;
			else
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeColorBG;

			glLineWidth( _drawer->getUIStyle()->NodeLineWidth );
			glColorPointer( 3, GL_FLOAT, 0, cols );
			glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

			glDrawArrays( GL_QUADS, 0, _verts.size() );

			if( _op && _currentDrawer && !_collapsed )
			{
				if( _op->getEnabled() )
					cols[0] = cols[1] = cols[2] = cols[3] = white();
				else
					cols[0] = cols[1] = cols[2] = cols[3] = grey();

				glVertexPointer( 2, GL_FLOAT, 0, &_mapVerts[0] );
				glColorPointer( 3, GL_FLOAT, 0, cols );

				glEnableClientState( GL_TEXTURE_COORD_ARRAY );

				glTexCoordPointer( 2, GL_FLOAT, 0, &_mapUVs[0] );

				glEnable( GL_TEXTURE_2D );
				glBindTexture( GL_TEXTURE_2D, _currentDrawer->getRTName() );

				glDrawArrays( GL_QUADS, 0, _mapVerts.size() );

				glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				glDisable( GL_TEXTURE_2D );
			}
		}
		glPopAttrib();
		glPopMatrix();

		glm::vec4 textPos( _drawer->getMV() * glm::vec4( _pos.x + _drawer->getUIStyle()->NodePaddingX, _pos.y + _drawer->getUIStyle()->TextDY, 0, 1 ) );
		const char *name = ( _op->getCustomName().length() ? _op->getCustomName().c_str() : _op->getName() );
		_drawer->getUIStyle()->getFont().print( name, textPos.x, textPos.y, _drawer->getWindowSize().x, _drawer->getWindowSize().y, _drawer->getMV()[2][2], _drawer->getUIStyle()->TextColor );

		//if( _sgd && _op && _sgd->getDrawDebug() )
		//{
		//	textPos = App().getMVCanvas() * glm::vec4( _pos.x, _pos.y, 0, 1 );
		//	char tempStr[64];
		//	sprintf( tempStr, "#%02zd", _op->getDebugID() );
		//	_drawer->getUIStyle()->getFont().print( tempStr, textPos.x, textPos.y, App().getWindowSize().x, App().getWindowSize().y, 1.0f, _drawer->getUIStyle()->TextColor );
		//}

		return true;
	}

	bool OpDrawer::drawUI()
	{
		bool ret = NodeDrawer::drawUI();

		//push ID so equally-named elements don't override cross-node when switching between nodes of same type
		ImGui::PushID( guidToString( _op->getObjectID() ).c_str() );

		if( ret )
		{
			if( _op )
			{
				try
				{
					_op->drawUI();
				}
				catch( std::exception &e )
				{
					std::cerr << "<error> caught exception drawing UI of " << _op->getName() << ", " << guidToString( _op->getClassID() ) << " (obj " << guidToString( _op->getObjectID() ) << "):" << std::endl
						<< e.what() << std::endl
						<< "deactivating Op" << std::endl;

					_op->setEnabled( false );
					setSelected( false );
				}
			}

			if( ImGui::TreeNode( "drawing" ) )
			{
				for( auto it : _drawers )
					if( ImGui::RadioButton( it->getName(), _currentDrawer == it ) )
						_currentDrawer = it;

				if( _currentDrawer )
					_currentDrawer->drawUI();

				ImGui::TreePop();
			}
		}

		ImGui::PopID();

		return ret;
	}

	bool OpDrawer::mouseDown( int button, int mods, bool imGuiHandled )
	{
		bool ret = NodeDrawer::mouseDown( button, mods, imGuiHandled );

		for( auto it : _buttons )
			ret |= it->mouseDown( button, mods, imGuiHandled );
		for( auto it : _inlets )
			ret |= it->mouseDown( button, mods, imGuiHandled );
		for( auto it : _outlets )
			ret |= it->mouseDown( button, mods, imGuiHandled );

		return ret;
	}

	bool OpDrawer::mouseUp( int button, int mods, bool imGuiHandled )
	{
		bool ret = NodeDrawer::mouseUp( button, mods, imGuiHandled );

		for( auto it : _buttons )
			ret |= it->mouseUp( button, mods, imGuiHandled );
		for( auto it : _inlets )
			ret |= it->mouseUp( button, mods, imGuiHandled );
		for( auto it : _outlets )
			ret |= it->mouseUp( button, mods, imGuiHandled );

		return ret;
	}

	float OpDrawer::setZ( float z )
	{
		z = GUI::Element::setZ( z );

		for( auto it : _inlets )
			z = it->setZ( z + 0.01f );
		for( auto it : _outlets )
			z = it->setZ( z + 0.01f );
		for( auto it : _buttons )
			z = it->setZ( z + 0.01f );

		return z;
	}

	bool OpDrawer::saveToJSON( nlohmann::json &j ) const
	{
		bool ret = NodeDrawer::saveToJSON( j );

		if( _op )
		{
			save( j, "classID", _op->getClassID() );
			save( j, "objectID", _op->getObjectID() );
		}

		save( j, "collapsed", _collapsed );

		if( _currentDrawer )
			save( j, "activeDrawerID", _currentDrawer->getClassID() );

		int cntr = 0;
		nlohmann::json ds = nlohmann::json::array();;
		for( auto it : _drawers )
		{
			nlohmann::json d;

			if( it->saveToJSON( d ) )
				ds[cntr++] = d;
		}
		j["drawers"] = ds;

		return ret;
	}

	bool OpDrawer::loadFromJSON( const nlohmann::json &j )
	{
		bool ret = NodeDrawer::loadFromJSON( j );

		load<bool>( j, "collapsed", _collapsed );
		_collapseButton->setActive( _collapsed );
		rebuild();

		nlohmann::json ds = j["drawers"];
		if( !ds.is_null() )
		{
			for( auto it = ds.begin(); it != ds.end(); ++it )
			{
				nlohmann::json d = it.value();

				std::string name;
				std::string classID;

				load<std::string>( d, "name", name );
				load<std::string>( d, "classID", classID );

				FrameDrawer *drawer = getDrawer( guidFromString( classID ) );

				if( drawer )
				{
					if( !drawer->loadFromJSON( d ) )
						std::cerr << "<error> failed to initialize drawer " << name << " from JSON file" << std::endl;
				}
				else
					std::cerr << "<error> drawer " << name << " (" << classID << " not found" << std::endl;
			}
		}

		auto did = j.find( "activeDrawerID" );
		if( did != j.end() )
		{
			std::string drawerID( did->get<std::string>() );
			_currentDrawer = getDrawer( guidFromString( drawerID ) );

			if( !_currentDrawer )
				std::cerr << "<error> drawer with ID " << drawerID << " not found" << std::endl;
		}
		else if( _drawers.size() )
			_currentDrawer = _drawers.front();

		return ret;
	}

	FrameDrawer *OpDrawer::getDrawer( const GUID &did ) const
	{
		for( auto it : _drawers )
			if( did == it->getClassID() )
				return it;

		return nullptr;
	}

	bool OpDrawer::update( const SampleFrame *sf ) const
	{
		if( _currentDrawer )//&& !_collapsed )
			return _currentDrawer->update( sf );
		return false;
	}

	void OpDrawer::setMaximized( bool maximized )
	{
		_maximized = maximized;
		_maximizeButton->setActive( maximized );
	}

	const InletPinDrawer *OpDrawer::getInlet( const std::string &name ) const
	{
		auto it = _inletsMap.find( name );
		if( it == _inletsMap.end() )
			return nullptr;
		return it->second;
	}

	const OutletPinDrawer *OpDrawer::getOutlet( const std::string &name ) const
	{
		auto it = _outletsMap.find( name );
		if( it == _outletsMap.end() )
			return nullptr;
		return it->second;
	}

	void OpDrawer::collapseButtonCallback( const GUI::Button *button, GUI::Button::Event e )
	{
		switch( e )
		{
		case GUI::Button::E_PRESSED:
			_collapsed = button->getActive();
			rebuild();
			break;
		case GUI::Button::E_RELEASED:
			break;
		}
	}

	void OpDrawer::maximizeButtonCallback( const GUI::Button *button, GUI::Button::Event e )
	{
		switch( e )
		{
		case GUI::Button::E_PRESSED:
		{
			SceneGraphDrawer *sgd = dynamic_cast<SceneGraphDrawer*>( _drawer );
			if( sgd )
			{
				if( button->getActive() )
					sgd->setMaximized( this );
				else
					sgd->setMaximized( nullptr );
			}
			break;
		}
		case GUI::Button::E_RELEASED:
			break;
		}
	}

	bool OpDrawer::childHit( const glm::vec2 &mousePos )
	{
		bool ret = NodeDrawer::childHit( mousePos );

		for( auto it : _inlets )
			ret |= it->hitTest( mousePos );
		for( auto it : _outlets )
			ret |= it->hitTest( mousePos );
		for( auto it : _buttons )
			ret |= it->hitTest( mousePos );

		return ret;
	}
}
#endif