/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "sink.h"

#include "../sceneGraph.h"

#include <app.h>
#include <fileIO/json.h>

#include <commonImGui.h>

#include "../../interfaces/serialMsg.h"

namespace sqid
{
	DEFINE_OP_DESC( Sink, "sink", "/devices",
		"F5057C23-768F-49B3-B9D5-12CC7DC6D3F2" );


	Sink::Sink( unsigned short port, unsigned int maxBufferSize ) :
		Op(),
		_interface( DIT_COUNT ),
		_port( port ),
		_deviceID( -1 ),
		_sensorID( -1 ),
		_msgOSC( "/" ),
		_inputBufferMsg( 128 ),
		_lastUpdateTime( 0 ),
		_sampleRate( 0.0f ),
		_sampleCntr( 0 ),
		_frameRate( 0.0f ),
		_frameCntr( 0 ),
		_dataRate( 0 ),
		_dataCntr( 0 ),
		_statsTimeAccu( 0.0f ),
		_maxBufferSize( maxBufferSize )
	{
		updateMsgFilter();
	}

	Sink::~Sink()
	{
		for( auto& it : _bufferedFrames )
			safeDelete( it );
		_bufferedFrames.clear();
	}

	void Sink::createPins()
	{
		addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
	}

	void Sink::queue( const SampleFrame* sf )
	{
		_sampleRate += sf->size();
		_frameRate++;
		_dataCntr += sf->size() * sizeof( float );

		_lastUpdateTime = getAppTime();

		_bufferedFrames.push_back( new SampleFrame( *sf ) );
		while( _bufferedFrames.size() > _maxBufferSize )
		{
			std::cout << "<warning> buffer size exceeded, dropping frames (sink)" << std::endl;

			SampleFrame* temp = _bufferedFrames.front();
			safeDelete( temp );

			_bufferedFrames.pop_front();
		}
	}

	bool Sink::fetchFrames( std::vector<SampleFrameContainer>& frames )
	{
		if( frames.size() )
			std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory!" << std::endl;

		if( _bufferedFrames.size() )
		{
			for( auto& it : _bufferedFrames )
			{
				if( _interface == DIT_COM || _interface == DIT_RFCOMM )
					frames.push_back( SampleFrameContainer( _deviceID, _sensorID, it ) );
				//else if( _interface == DIT_OSC )
				//	frames.push_back( SampleFrameContainer( _msgOSC, it ) );
				else
				{
					std::cerr << "<error> invalid data interface type" << std::endl;
					safeDelete( it );
				}
			}

			_bufferedFrames.clear();

			return true;
		}

		return false;
	}

	bool Sink::process()
	{
		SampleFrame* sf = fetchInput<SampleFrame>( "in" );
		if( sf )
		{
			queue( sf );

			drawFrame( sf );

			safeDelete( sf );
		}

		return inputPending( "in" );
	}

	void Sink::updateMsgFilter()
	{
		strncpy( &_inputBufferMsg[0], _msgOSC.c_str(), _inputBufferMsg.size() );
	}

	void Sink::updateStats( float dt )
	{
		_statsTimeAccu += dt;
		if( _statsTimeAccu > 1.0f )
		{
			_sampleRate = _sampleRate / _statsTimeAccu;
			_frameRate = _frameCntr / _statsTimeAccu;
			_dataRate = _dataCntr / _statsTimeAccu;

			_sampleCntr = 0;
			_frameCntr = 0;
			_dataCntr = 0;

			_statsTimeAccu = 0.0f;
		}
	}

#ifdef __SUPPORT_GUI
	bool Sink::drawUI()
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
					_interface = (DataInterfaceType) i;
				}
				if( isSelected )
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if( _interface == DIT_OSC )
		{
			std::cerr << "<warning> OSC not supported yet, use oscOut Operator instead" << std::endl;
			_interface = DIT_COUNT;
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
			ScopedImGuiStyleColor redText( ImGuiCol_Text, ImVec4( 1, 0, 0, 1 ), strcmp( _msgOSC.c_str(), &_inputBufferMsg[0] ) );
			if( ImGui::InputText( "msg filter", &_inputBufferMsg[0], _inputBufferMsg.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				_msgOSC = std::string( &_inputBufferMsg[0] );
			//ImGui::Checkbox( "exact match", &_msgOSC );
		}

		ImGui::Text( "%.02f sps", _sampleRate );
		ImGui::Text( "%.02f fps", _frameRate );
		ImGui::Text( "%.02f kbps", ( _dataRate << 3 ) / 1024.0f );
		//if( _sinkDesc.size() )
		//	ImGui::Text( _sinkDesc.c_str() );

		return true;
	}
#endif

	bool Sink::loadFromJSON( const nlohmann::json& j )
	{
		bool ret = Op::loadFromJSON( j );

		std::string str;
		if( load<std::string>( j, "interface", str ) )
			_interface = interfaceFromString( str );
		load<unsigned short>( j, "port", _port );

		load<unsigned char>( j, "deviceID", _deviceID );
		load<unsigned char>( j, "sensorID", _sensorID );

		load<std::string>( j, "msg", _msgOSC );
		//load<bool>( j, "exact", _exactOSC );

		updateMsgFilter();

		return ret;
	}

	bool Sink::saveToJSON( nlohmann::json& j ) const
	{
		bool ret = Op::saveToJSON( j );

		save( j, "interface", interfaceToString( _interface ) );
		save( j, "port", _port );

		save( j, "deviceID", (int) _deviceID );
		save( j, "sensorID", (int) _sensorID );

		save( j, "msg", _msgOSC );
		//save( j, "exact", _exactOSC );

		return ret;
	}
}
