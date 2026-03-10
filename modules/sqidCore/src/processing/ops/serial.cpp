/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "serial.h"

#include <fileIO/json.h>

#include "../sceneGraph.h"
#include "../../interfaces/serialMsg.h"

#include <commonImGui.h>

#include <serial/serial.h>

#include <opencv2/imgproc.hpp>


namespace sqid
{
	namespace Serial
	{
		//deprecated -> use COM/serial interface in combination with Sink op
		DEFINE_OP_DESC( SerialOut, "serialOut", "/deprecated/devices",
			"4EBAA930-4589-4E61-BB95-06A60BC05268" );

		SerialOut::SerialOut() :
			Op(),
			_sending( false ),
			_baud( 115200 ),
			//_queueSize( 16 ),
			_deviceID( -1 ),
			_sensorID( -1 ),
			_dropdownSelected( ~0x00 ),
			_baudUI( 115200 )
		{
			if( !SerialPort::isInitialized() )
				if( !SerialPort::init() )
					std::cerr << "<error> failed to initialize serial port" << std::endl;
		}

		SerialOut::~SerialOut()
		{
			close();
		}

		bool SerialOut::process()
		{
			//test
			/*
			if( _port.isOpen() )
			{
				size_t available = _port.available();
				if( available )
				{
					std::vector<char> data( available + 1 );
					data[available] = 0;
					_port.read( reinterpret_cast<uint8_t*>( &data[0] ), available );
					std::cout << &data[0];
				}
			}
			*/
			//----

			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				if( _sending )
				{
					if( !_port.isOpen() )
						_sending = false;
					else
					{
						

						bool err = false;
						MsgType type = typeFromFrame( sf );

						//TODO: implement compression
						//TODO: implement quantization and normalization to, e.g., 10-bit or 12-bit int values
						MsgFlags flags = (MsgFlags)( MF_NORMALIZED | MF_DATATYPE_FLOAT | MF_ENC_UNCOMPRESSED );
						size_t hdrSize = 0;
						size_t elements = sf->width() * sf->height() * sf->depth();
						size_t payloadSize = elements * sizeof( float );
						switch( type )
						{
						case MT_VALUE:
						{
							hdrSize = sizeof( DataHdrSingleValue );
							break;
						}
						case MT_ARRAY:
						{
							hdrSize = sizeof( DataHdrArray );
							break;
						}
						case MT_MATRIX:
						{
							hdrSize = sizeof( DataHdrMatrix );
							break;
						}
						case MT_IMAGE:
						{
							hdrSize = sizeof( DataHdrImage );
							break;
						}
						default:
						{
							std::cerr << "<error> invalid type" << std::endl;
							err = true;
						}
						}

						if( hdrSize + payloadSize > 0xffff )
						{
							std::cerr << "<error> frame size exceeds max msg size" << std::endl;
							err = true;
						}

						std::vector<uint8_t> data( hdrSize + payloadSize );
						switch( type )
						{
						case MT_VALUE:
						{
							DataHdrSingleValue *dh = reinterpret_cast<DataHdrSingleValue*>( &data[0] );
							dh->flags = flags;
							break;
						}
						case MT_ARRAY:
						{
							DataHdrArray *dh = reinterpret_cast<DataHdrArray*>( &data[0] );
							dh->flags = flags;
							dh->size = elements;
							break;
						}
						case MT_MATRIX:
						{
							DataHdrMatrix *dh = reinterpret_cast<DataHdrMatrix*>( &data[0] );
							dh->flags = flags;
							dh->width = sf->width();
							dh->height = sf->height();
							break;
						}
						case MT_IMAGE:
						{
							DataHdrImage* dh = reinterpret_cast<DataHdrImage*>( &data[0] );
							dh->flags = flags;
							dh->width = sf->width();
							dh->height = sf->height();
							dh->depth = sf->depth();
							break;
						}
						default:
						{
							std::cerr << "<error> invalid type" << std::endl;
							err = true;
						}
						}

						memcpy( &data[hdrSize], sf->values(), payloadSize );

						ComMsg msg;
						msg.hdr.hdr =
						{
							ProtocolVersion::PV_2,
							type,
							_deviceID,
							_sensorID,
							sf->timeStamp(),
							(uint16_t)( hdrSize + payloadSize )
						};
						msg.hdr.chk = makeChkSum( &msg.hdr.hdr );
						msg.data = &data[0];

						if( !err )
						{
							if( _port.write( (uint8_t*) syncBytes, SYNC_BYTES ) != SYNC_BYTES )
								err = true;
							if( _port.write( (uint8_t*) &msg.hdr, sizeof( ComMsgHdrEx ) ) != sizeof( ComMsgHdrEx ) )
								err = true;
							if( _port.write( (uint8_t*) msg.data, hdrSize + payloadSize ) != hdrSize + payloadSize )
								err = true;

							if( err ) 
								std::cerr << "<error> sending data to " << _portName << " failed" << std::endl;
						}

						if( err )
							_sending = false;
						else
							drawFrame( sf );
					}
				}
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool SerialOut::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _sending ? "stop" : "start" ) )
			{
				auto &devices = SerialPort::getKnownPorts();

				if( _sending )
				{
					_sending = false;
					close();
				}
				else if( _dropdownSelected < devices.size() )
				{
					_sending = true;
					open();
				}
			}

			{
				ScopedImGuiDisable disable( _sending );

				auto &devices = SerialPort::getKnownPorts();
				std::vector<std::string> items;
				for( auto &it : devices )
					items.push_back( it );

				if( !items.size() )
					items.push_back( "<none>" );

				if( _dropdownSelected >= items.size() )
				{
					_dropdownSelected = items.size() - 1;
					if( _dropdownSelected >= 0 )
						_portName = devices[_dropdownSelected];
					else
						_portName = "";
				}

				const char *currentItem = items[_dropdownSelected].c_str();

				if( ImGui::BeginCombo( "##combo", currentItem, ImGuiComboFlags_None ) )
				{
					for( int i = 0; i < items.size(); i++ )
					{
						bool isSelected = ( i == _dropdownSelected );
						if( ImGui::Selectable( items[i].c_str(), isSelected ) )
						{
							_dropdownSelected = i;
							_portName = items[i];
						}
						if( isSelected )
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if( ImGui::Button( "rescan" ) )
					SerialPort::rescan();

				int baud = _baudUI;
				if( ImGui::InputInt( "baud rate", &baud ) )
					_baudUI = baud;

				int i = _deviceID;
				if( ImGui::InputInt( "device ID", &i ) )
					_deviceID = i;
				i = _sensorID;
				if( ImGui::InputInt( "sensor ID", &i ) )
					_sensorID = i;
			}

			return true;
		}
#endif

		void SerialOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

		bool SerialOut::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "sending", _sending );
			load<std::string>( j, "portName", _portName );
			load<uint32_t>( j, "baud", _baud );
			load<unsigned char>( j, "deviceID", _deviceID );
			load<unsigned char>( j, "sensorID", _sensorID );

			_baudUI = _baud;

			_dropdownSelected = ~0x00;

			unsigned short portNr = getPortNr( _portName );
			auto &devices = SerialPort::getKnownPorts();
			if( portNr != ~0x00 )
				for( int i = 0; i < devices.size(); i++ )
					if( getPortNr( devices[i] ) == portNr )
						_dropdownSelected = i;

			if( _sending )
				open();

			return ret;
		}

		bool SerialOut::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "sending", _sending );
			save( j, "portName", _portName );
			save( j, "baud", _baud );
			save( j, "deviceID", _deviceID );
			save( j, "sensorID", _sensorID );

			return ret;
		}

		bool SerialOut::open()
		{
			close();

			if( !_port.open( _portName, _baud ) )
			{
				_sending = false;

				std::cerr << "failed to connect to serial device" << std::endl;
				return false;
			}

			return true;
		}

		void SerialOut::close()
		{
			_port.close();
		}
	}
}