/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "sensor.h"

#include "../sceneGraph.h"

#include <app.h>
#include <fileIO/json.h>

#include <commonImGui.h>

namespace sqid
{
	DEFINE_OP_DESC( Sensor, "sensor", "/devices",
		"40DE5B8D-4FCF-4A69-A65C-EBCC94D86034" );


	Sensor::Sensor( unsigned short port, float sensorTimeout, unsigned int maxBufferSize ) :
		Op(),
		_interface( DIT_COUNT ),
		_port( port ),
		_deviceID( -1 ),
		_sensorID( -1 ),
		_msgFilterOSC( "/" ),
		_exactOSC( false ),
		_inputBufferMsg( 128 ),
		_sensorTimeout( sensorTimeout ),
		_lastUpdateTime( 0 ),
		_inputRate( 0.0f ),
		_inputCntr( 0 ),
		_sampleRate( 0.0f ),
		_sampleCntr( 0 ),
		_dataRate( 0 ),
		_dataCntr( 0 ),
		_statsTimeAccu( 0.0f ),
		_maxBufferSize( maxBufferSize )
	{
		updateMsgFilter();
	}

	Sensor::~Sensor()
	{
		for( auto &it : _bufferedFrames )
			safeDelete( it );
		_bufferedFrames.clear();
	}

	void Sensor::createPins()
	{
		addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
	}

	bool Sensor::process()
	{
		if( _bufferedFrames.size() )
		{
			for( auto &it : _bufferedFrames )
			{
				//draw only last one
				if( it == _bufferedFrames.back() )
					drawFrame( it );

				pushOutput( "out", it );

				safeDelete( it );
			}
			_bufferedFrames.clear();
		}

		return false;
	}

	void Sensor::updateMsgFilter()
	{
		strncpy( &_inputBufferMsg[0], _msgFilterOSC.c_str(), _inputBufferMsg.size() );
	}

	bool Sensor::doesWant( const SampleFrameContainer *sfc, const std::string &senderDesc )
	{
		//NOTE: senderDesc is ignored for now, may be useful?
		if( _interface == DIT_COM || _interface == DIT_RFCOMM )
			return ( sfc->deviceID == _deviceID && sfc->sensorID == _sensorID );
		else if( _interface == DIT_OSC )
			return ( _exactOSC ? sfc->message.compare( _msgFilterOSC ) == 0 : sfc->message.compare( 0, _msgFilterOSC.size(), _msgFilterOSC ) == 0 );
		return false;
	}

	bool Sensor::feed( const SampleFrame *sf )
	{
		if( !sf )
			return false;

		_inputCntr++;
		_dataCntr += sf->size() * sizeof( float );

		_lastUpdateTime = getAppTime();

		_bufferedFrames.push_back( new SampleFrame( *sf ) );
		while( _bufferedFrames.size() > _maxBufferSize )
		{
			std::cout << "<warning> buffer size exceeded, dropping frames " << std::endl;

			SampleFrame *temp = _bufferedFrames.front();
			safeDelete( temp );

			_bufferedFrames.pop_front();
		}

		return true;
	}

	void Sensor::updateStats( float dt )
	{
		_statsTimeAccu += dt;
		if( _statsTimeAccu > 1.0f )
		{
			_inputRate = _inputCntr / _statsTimeAccu;
			_sampleRate = _sampleCntr / _statsTimeAccu;
			_dataRate = _dataCntr / _statsTimeAccu;

			_inputCntr = 0;
			_sampleCntr = 0;
			_dataCntr = 0;

			_statsTimeAccu = 0.0f;
		}
	}

	bool Sensor::isOffline()
	{
		if( _sensorTimeout < 0 )
			return false;
		return ( getAppTime() - _lastUpdateTime ) > _sensorTimeout;
	}

#ifdef __SUPPORT_GUI
	bool Sensor::drawUI()
	{
		if( !Op::drawUI() )
			return false;

		auto items = getDataInterfaceComboItems();
		items.push_back( "<none>" );
		int index = (int) _interface;

		const char* currentItem = items[index];
		if( ImGui::BeginCombo( "##combo", currentItem, ImGuiComboFlags_None ) )
		{
			for( int i = 0; i < items.size(); i++ )
			{
				bool isSelected = ( currentItem == items[i] );
				if( ImGui::Selectable( items[i], isSelected ) )
				{
					currentItem = items[i];
					_interface = (DataInterfaceType)i;
				}
				if( isSelected )
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		int i = _port;
		//TODO: color text red as long as input is not applied with Return (e.g., use ScopedImGuiStyleColor, see oscOut)
		if( ImGui::InputInt( "port", &i, 1, 16, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AlwaysOverwrite ) )
			_port = i;

		if( _interface == DIT_COM || _interface == DIT_RFCOMM )
		{
			i = _deviceID;
			//TODO: color text red as long as input is not applied with Return (e.g., use ScopedImGuiStyleColor, see oscOut)
			if( ImGui::InputInt( "deviceID", &i, 1, 16, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AlwaysOverwrite ) )
				_deviceID = i % 256;

			i = _sensorID;
			//TODO: color text red as long as input is not applied with Return (e.g., use ScopedImGuiStyleColor, see oscOut)
			if( ImGui::InputInt( "sensorID", &i, 1, 16, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AlwaysOverwrite ) )
				_sensorID = i % 256;
		}
		else if( _interface == DIT_OSC )
		{
			ScopedImGuiStyleColor redText( ImGuiCol_Text, ImVec4( 1, 0, 0, 1 ), strcmp( _msgFilterOSC.c_str(), &_inputBufferMsg[0] ) );
			if( ImGui::InputText( "msg filter", &_inputBufferMsg[0], _inputBufferMsg.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				_msgFilterOSC = std::string( &_inputBufferMsg[0] );
			ImGui::Checkbox( "exact match", &_exactOSC );
		}

		ImGui::Text( "%.02f sps (in)", _inputRate );
		ImGui::Text( "%.02f sps (out)", _sampleRate );
		ImGui::Text( "%.02f kbps", ( _dataRate << 3 ) / 1024.0f );
		if( _sourceDesc.size() )
			ImGui::Text( _sourceDesc.c_str() );

		return true;
	}
#endif

	bool Sensor::loadFromJSON( const nlohmann::json &j )
	{
		bool ret = Op::loadFromJSON( j );

		std::string str;
		if( load<std::string>( j, "interface", str ) )
			_interface = interfaceFromString( str );
		load<unsigned short>( j, "port", _port );

		load<unsigned char>( j, "deviceID", _deviceID );
		load<unsigned char>( j, "sensorID", _sensorID );

		load<std::string>( j, "msgFilter", _msgFilterOSC );
		load<bool>( j, "exact", _exactOSC );

		updateMsgFilter();

		return ret;
	}

	bool Sensor::saveToJSON( nlohmann::json &j ) const
	{
		bool ret = Op::saveToJSON( j );

		save( j, "interface", interfaceToString( _interface ) );
		save( j, "port", _port );

		save( j, "deviceID", (int) _deviceID );
		save( j, "sensorID", (int) _sensorID );

		save( j, "msgFilter", _msgFilterOSC );
		save( j, "exact", _exactOSC );

		return ret;
	}
}
