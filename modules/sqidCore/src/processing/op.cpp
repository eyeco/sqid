/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <processing/op.h>

#include <processing/pin.h>
#ifdef __SUPPORT_GUI
#include <drawing/frameDrawer.h>
#include "../drawing/sceneUI/opDrawer.h"
#endif

#include "sceneGraph.h"

#include <fileIO/json.h>

#ifdef __SUPPORT_GUI
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#endif

#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>



#include <chrono>
#include <iostream>


#define OP_CUSTOMNAME_MAXLEN	256

namespace sqid
{
	Op::Op() :
		_objectID( randomGUID() ),
		_processingTime( 0 ),
		_dbgID( -1 ),
#ifdef __SUPPORT_GUI
		_drawer( nullptr ),
#endif
		_isEnabled( true )
	{
		_customName[0] = 0;
	}

	Op::~Op()
	{
		for( auto &it : _inlets )
			safeDelete( it.second );
		_inlets.clear();

		for( auto &it : _outlets )
			safeDelete( it.second );
		_outlets.clear();
	}

#ifdef __SUPPORT_GUI
	std::vector<FrameDrawer*> Op::createDrawers()
	{
		std::vector<FrameDrawer*> drawers;

		drawers.push_back( new FrameDrawer2D() );
		drawers.push_back( new FrameDrawerLines() );
		drawers.push_back( new HistoryDrawerLines() );
		drawers.push_back( new HistoryDrawerMap() );

		return drawers;
	}
#endif

	void Op::createPins()
	{
		addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
	}

	void Op::deletePins()
	{
		for( auto &it : _inlets )
			safeDelete( it.second );
		_inlets.clear();

		for( auto &it : _outlets )
			safeDelete( it.second );
		_outlets.clear();
	}

	InletPin *Op::addInlet( InletPin *inlet )
	{
		if( !inlet )
			return nullptr;

		if( _inlets.find( inlet->getName() ) != _inlets.end() )
		{
			std::cerr << "<error> inlet with name \"" << inlet->getName() << "\" already present" << std::endl;
			delete( inlet );

			return nullptr;
		}

		_inlets.insert( std::make_pair( inlet->getName(), inlet ) );
		return inlet;
	}

	OutletPin *Op::addOutlet( OutletPin *outlet )
	{
		if( !outlet )
			return nullptr;

		if( _outlets.find( outlet->getName() ) != _outlets.end() )
		{
			std::cerr << "<error> outlet with name \"" << outlet->getName() << "\" already present" << std::endl;
			delete( outlet );

			return nullptr;
		}

		_outlets.insert( std::make_pair( outlet->getName(), outlet ) );
		return outlet;
	}

	InletPin *Op::getInlet( const std::string &name ) const
	{
		auto it = _inlets.find( name );
		if( it == _inlets.end() )
			return nullptr;
		return it->second;
	}

	OutletPin *Op::getOutlet( const std::string &name ) const
	{
		auto it = _outlets.find( name );
		if( it == _outlets.end() )
			return nullptr;
		return it->second;
	}

	bool Op::inputPending( const std::string &inletName ) const
	{
		auto it = _inlets.find( inletName );
		if( it == _inlets.end() )
		{
			std::cerr << "<warning> tried to fetch unknown input \"" << inletName << "\"" << std::endl;
			return 0;
		}

		return ( it->second->getPending() != 0 );
	}
	
	bool Op::update()
	{
		bool ret = false;

		for( auto &it : _outlets )
			it.second->resetActivity();

		if( _isEnabled )
		{
			using namespace std::chrono;

			high_resolution_clock::time_point t0 = high_resolution_clock::now();

			try
			{
				while( process() )
				{}
			}
			catch( std::exception &e )
			{
				std::cerr << "<error> caught exception processing " << getName() << ", " << guidToString( getClassID() ) << " (obj " << guidToString( getObjectID() ) << "):" << std::endl
					<< e.what() << std::endl
					<< "deactivating Op" << std::endl;

				setEnabled( false );
			}

			duration<double> timeSpan = duration_cast<duration<double>>( high_resolution_clock::now() - t0 );

			//TODO: subtract time spent to draw frame
			_processingTime = (unsigned int) ( timeSpan.count() * 1e6 );
		}

		for( auto &it : _inlets )
			it.second->resetActivity();

		for( auto &it : _inlets )
			it.second->clear();
		for( auto &it : _outlets )
			it.second->clear();

		return ret;
	}

#ifdef __SUPPORT_GUI
	bool Op::drawFrame( const SampleFrame *sf )
	{
		bool ret = true;

		if( !sf || !_drawer )
			return false;

		return _drawer->update( sf );
	}

	bool Op::drawUI()
	{
		ImGui::Text( this->getName() );

		char name[OP_CUSTOMNAME_MAXLEN];
		strcpy_s( name, _customName.c_str() );
		if( ImGui::InputText( "name", name, OP_CUSTOMNAME_MAXLEN ) )
			_customName = std::string( name );

		//TODO: draw all UI elements in inspector disabled, when this checkbox isn't set
		ImGui::Checkbox( "enabled", &_isEnabled );

		char tempStr[128];
		sprintf( tempStr, "pt: %d us", _processingTime );
		ImGui::Text( tempStr );

		return true;
	}
#endif

	bool Op::loadFromJSON( const nlohmann::json &j )
	{
#ifdef _DEBUG
		std::string name;
		load<std::string>( j, "name", name );
		if( strcmp( getName(), name.c_str() ) )
			std::cerr << "<warning> name does not match (" << getName() << ": " << name << " should be " << getName() << ")" << std::endl;
		std::string path;
		load<std::string>( j, "path", path );
		if( strcmp( getPath(), path.c_str() ) )
			std::cerr << "<warning> path does not match (" << getName() << ": " << path << " should be " << getPath() << ")" << std::endl;
		std::string classID;
		load<std::string>( j, "classID", classID );
		if( getClassID() != guidFromString( classID ) )
			std::cerr << "<warning> ClassID does not match (" << getName() << ": " << classID << " should be " << guidToString( getClassID() ) << ")" << std::endl;
#endif

		std::string objID;
		load<std::string>( j, "objectID", objID );
		std::cout << "setting objID " << objID << std::endl;
		_objectID = guidFromString( objID );

		load<std::string>( j, "customName", _customName );
		load<bool>( j, "enabled", _isEnabled );

		return true;
	}

	bool Op::saveToJSON( nlohmann::json &j ) const
	{
		save( j, "name", getName() );
		save( j, "path", getPath() );
		save( j, "classID", getClassID() );

		save( j, "objectID", getObjectID() );
		
		save( j, "customName", getCustomName() );
		save( j, "enabled", _isEnabled );

		return true;
	}
}
