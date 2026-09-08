/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "comInterface.h"

#include "serialMsg.h"
#include "../ringBuffer.h"

#include <fileIO/json.h>
#include <commonImGui.h>

#include <map>
#include <list>
#include <mutex>
#include <thread>
#include <iostream>

#include <imgui/imgui.h>

#include "../serialPort.h"

#define COM_SOURCE_BAUD_DEFAULT	115200

namespace sqid
{
	namespace Internal
	{
		class COMInterfaceImpl// : public IAbstractWriter
		{
		private:
			enum ProtocolState
			{
				PS_SYNCING,
				PS_HEADER,
				PS_PAYLOAD,

				PS_COUNT
			};

			bool _doRead;
			bool _doWrite;

			bool _isSynced;
			ProtocolState _currentState;
			unsigned int _expectedBytes;

			unsigned int _maxQueueSize;

			std::mutex _readMsgMutex;
			std::list<ComMsg> _readMsgQueue;

			std::mutex _writeMsgMutex;
			std::list<ComMsg> _writeMsgQueue;

			std::set<uint16_t> _activeSenders;

			RingBuffer<unsigned char> _buffer;

			ComMsgHdrEx _hdrEx;
			std::vector<unsigned char> _dataBuffer;

			ComMsgParser _parser;

			bool _isStarted;

			bool _keepRunning;
			bool _tryReconnect;

			std::string _portName;
			unsigned int _baudRate;

			SerialPort _port;

			int _reconnectDelay;

			size_t _readBufferSize;

			std::thread *_comListenerThread;

			ComMsgBuilder _msgBuilder;

			bool tryConnect()
			{
				this->closeCOM();

				if( !_port.open( _portName, _baudRate ) )
				{
					std::cerr << "failed to connect com interface" << std::endl;

					this->closeCOM();
					return false;
				}

				if( !_port.isOpen() )
				{
					this->closeCOM();
					return false;
				}

				_isSynced = false;
				_currentState = PS_SYNCING;
				_expectedBytes = SYNC_BYTES;

				return true;
			}

			void comListen()
			{
				while( _tryReconnect )
				{
					if( !tryConnect() )
					{
						std::cout << "starting a reconnect attempt in " << _reconnectDelay << " seconds ..." << std::endl;

						for( int i = 0; i < _reconnectDelay * 10; i++ )
						{
							if( !_tryReconnect )
								break; // break if closed in the meantime

							std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
						}

						continue;
					}

					size_t bytesRead;

					std::vector<unsigned char> b( _readBufferSize );

					try
					{
						while( _keepRunning )
						{
							{
								std::lock_guard<std::mutex> lock( _writeMsgMutex );

								while( _writeMsgQueue.size() )
								{
									ComMsg &msg = _writeMsgQueue.front();

									writeSerial( (uint8_t*) syncBytes, SYNC_BYTES );						//send sync bytes
									writeSerial( (uint8_t*) &( msg.hdr ), (int) sizeof( ComMsgHdrEx ) );	//send header
									writeSerial( (uint8_t*) msg.data, msg.hdr.hdr.dataBytes );				//send data

									safeDeleteArray( msg.data );
									_writeMsgQueue.pop_front();
								}
							}

							if( !_port.available() )	//don't block here, otherwise writing to serial port will also be blocked
								std::this_thread::sleep_for( std::chrono::microseconds( 1 ) );
							else
							{
								bytesRead = _port.read( &b[0], b.size() );
								if( bytesRead == -1 )
								{
									if( _keepRunning )	//otherwise, we closed connection intentionally, everything is fine
										std::cerr << "<error> failed to read from serial port!" << std::endl;
									continue;
								}

								if( !bytesRead )
								{
									//std::cout << "no bytes read" << std::endl;

									//TODO: ReadFile frequently detects EOF and then returns with success and bytesRead == 0
									// which means ReadFile does not block in many cases and thus the code consumes unneccessarily 
									// high amount of CPU power. probably there is a recommended, better way of transferrig binary
									// data via serial, which would not seem to send EOFs all the time?
									//NOTE: since we moved to cross-platform serial library, this may no longer be an issue -- 
									//  doublecheck/test properly at some point
									continue;
								}

								_buffer.add( &b[0], bytesRead );
								if( _currentState == PS_SYNCING )
								{
									while( _buffer.size() >= _expectedBytes )
									{
										unsigned char bytes[SYNC_BYTES];
										_buffer.peek( bytes, SYNC_BYTES );

										if( !memcmp( syncBytes, bytes, SYNC_BYTES ) )
										{
											_buffer.get( bytes, SYNC_BYTES );

											if( !_isSynced )
											{
												_isSynced = true;
												std::cout << "synced port " << _portName << std::endl;
											}

											_currentState = PS_HEADER;
											_expectedBytes = sizeof( ComMsgHdrEx );

											break;
										}
										else
											_buffer.get();	// move forward by one byte
									}
								}

								if( _currentState == PS_HEADER )
								{
									if( _buffer.size() >= _expectedBytes )
									{
										_buffer.peek( reinterpret_cast<unsigned char*>( &_hdrEx ), sizeof( ComMsgHdrEx ) );

										if( _hdrEx.hdr.ver != PV_2 )
										{
											std::cerr << "<error> serial protocol " << _hdrEx.hdr.ver << " not supported (requiring PV" << PV_2 << "), stopping reading from interface..." << std::endl;
											
											_tryReconnect = false;
											_keepRunning = false;

											closeCOM();
										}

										if( !checkHdr( &_hdrEx ) )
										{
											std::cerr << "<error> checksum of header corrupt -- maybe out of sync, trying to resync..." << std::endl;
											//NOTE: ALSO CHECK YOUR STRUCT DATA PACKING ALIGNMENT AT RECEIVER AND SENDER SIDE IN THIS CASE!!

											_isSynced = false;
											_currentState = PS_SYNCING;
											_expectedBytes = SYNC_BYTES;
										}
										else
										{
											//discard previously peeked bytes
											_buffer.get( reinterpret_cast<unsigned char*>( &_hdrEx ), sizeof( ComMsgHdrEx ) );

											_currentState = PS_PAYLOAD;
											_expectedBytes = _hdrEx.hdr.dataBytes;

											if( _dataBuffer.size() < _expectedBytes )
												_dataBuffer.resize( nextPo2( _expectedBytes ) );
										}
									}
								}

								if( _currentState == PS_PAYLOAD )
								{
									if( _buffer.size() >= _expectedBytes )
									{
										_buffer.peek( &_dataBuffer[0], _expectedBytes );

										bool valid = true;
										//TODO: check data integrity
										if( !valid )
										{
											std::cerr << "<error> payload data seems to be corrupt -- maybe out of sync, trying to resync..." << std::endl;
											_isSynced = false;
											_currentState = PS_SYNCING;
											_expectedBytes = SYNC_BYTES;
										}
										else
										{
											//discard previously peeked bytes
											_buffer.get( &_dataBuffer[0], _expectedBytes );

											ComMsg msg = {};
											msg.hdr = _hdrEx;
											msg.data = new unsigned char[_expectedBytes];
											memcpy( msg.data, &_dataBuffer[0], _expectedBytes );

											/*if( msg.hdr.hdr.type == MT_PROPERTY )
											{
												const char *str = (char*) msg.data;

												size_t maxSize = strlen( str ) + 1;

												std::vector<std::string> subs = split( std::string( str ), '=' );
												if( subs.size() != 2 )
													std::cerr << "ill-formatted property string: \"" << str << "\" -- dropping it..." << std::endl;

												{
													std::lock_guard<std::mutex> lock( _propMutex );
													_propMap[subs[0]] = subs[1];
												}

												safeDeleteArray( msg.data );
											}
											else*/
											{
												std::lock_guard<std::mutex> lock( _readMsgMutex );

												//TODO: there was a mem-leak (32 bytes) reported around here somewhere, which I never had before, revisit this
												_readMsgQueue.push_back( msg );

												_activeSenders.insert( msg.hdr.hdr.deviceID << 8 | msg.hdr.hdr.sensorID );

												while( _readMsgQueue.size() > _maxQueueSize )
												{
													safeDeleteArray( _readMsgQueue.front().data );
													_readMsgQueue.pop_front();
												}
											}

											_currentState = PS_SYNCING;
											_expectedBytes = SYNC_BYTES;
										}
									}
								}
							}
						}
					}
					catch( std::exception &e )
					{
						std::cerr << "caught exception in comListen: " << e.what() << std::endl;
					}
				}
			}

			void closeCOM()
			{
				_currentState = PS_COUNT;
				_port.close();
			}

			bool writeSerial( const unsigned char* data, size_t size )
			{
				if( !_port.isOpen() )
					return false;

				return ( _port.write( data, size ) == size );
			}

		public:
			COMInterfaceImpl( const std::string &portName, unsigned int baudRate, unsigned int maxQueueSize ) :
				_doRead( true ),
				_doWrite( false ),
				_isSynced( false ),
				_currentState( PS_COUNT ),
				_expectedBytes( 0 ),
				_maxQueueSize( maxQueueSize ),
				_hdrEx( { 0 } ),
				_dataBuffer( 1024 ),
				_isStarted( false ),
				_keepRunning( false ),
				_tryReconnect( true ),
				_portName( portName ),
				_baudRate( baudRate ),
				_reconnectDelay( 5 ),
				_readBufferSize( 10 ),	//TODO: maybe it is reasonable, to couple this value to the baudRate?
				_comListenerThread( nullptr )
			{}

			~COMInterfaceImpl()
			{
				close();
			}

			bool run( IOMode mode )
			{
				//TODO: this query is not *entirely* threadsafe, but it should do...
				if( _isStarted )
					return false;
				_isStarted = true;

				if( _port.isOpen() )	//already running
					return false;

				_doRead = mode & IOM_INPUT;
				_doWrite = mode & IOM_OUTPUT;

				if( !_doRead && !_doWrite )
				{
					std::cerr << "<error> invalid IOMode" << (int) mode << std::endl;
					return false;
				}

				_keepRunning = true;
				_comListenerThread = new std::thread( &COMInterfaceImpl::comListen, this );

				return true;
			}

			void close()
			{
				_keepRunning = false;
				_tryReconnect = false;

				this->closeCOM();

				if( _comListenerThread )
				{
					_comListenerThread->join();

					safeDelete( _comListenerThread );
				}

				{
					std::lock_guard<std::mutex> lock( _readMsgMutex );

					for( auto it = _readMsgQueue.begin(); it != _readMsgQueue.end(); ++it )
						safeDeleteArray( it->data );
					_readMsgQueue.clear();

					_activeSenders.clear();
				}

				{
					std::lock_guard<std::mutex> lock( _writeMsgMutex );

					for( auto it = _writeMsgQueue.begin(); it != _writeMsgQueue.end(); ++it )
						safeDeleteArray( it->data );
					_writeMsgQueue.clear();
				}
			}

			std::string getDesc() const
			{
				return _parser.getDesc();
			}

			void fetchFrames( std::vector<SampleFrameContainer> &frames )
			{
				if( frames.size() )
					std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory!" << std::endl;

				if( _readMsgQueue.size() )
				{
					std::lock_guard<std::mutex> lock( _readMsgMutex );

					while( _readMsgQueue.size() )
					{
						ComMsg &msg = _readMsgQueue.front();
						SampleFrame *frame = nullptr;

						switch( msg.hdr.hdr.type )
						{
						case MT_VALUE:
							frame = _parser.createFrameFromSingleValue( msg.hdr.hdr, msg.data );
							break;
						case MT_ARRAY:
							frame = _parser.createFrameFromArray( msg.hdr.hdr, msg.data );
							break;
						case MT_MATRIX:
							frame = _parser.createFrameFromMatrix( msg.hdr.hdr, msg.data );
							break;
						case MT_IMAGE:
							frame = _parser.createFrameFromImage( msg.hdr.hdr, msg.data );
							break;
						default:
							std::cerr << "<error> unknown type: " << msgTypeToString( msg.hdr.hdr.type ) << " (" << msg.hdr.hdr.type << ")" << std::endl;
							break;
						}

						if( frame )
							frames.push_back( SampleFrameContainer( msg.hdr.hdr.deviceID, msg.hdr.hdr.sensorID, frame ) );
						else
							std::cerr << "<error> failed to create frame" << std::endl;

						safeDeleteArray( msg.data );

						_readMsgQueue.pop_front();
					}
				}
			}

			bool write( const SampleFrame* frame, unsigned char deviceID, unsigned char sensorID, MsgFlags flags, bool clamp )
			{
				if( !_port.isOpen() || !_doWrite )
					return false;

				const ComMsg* msg = _msgBuilder.build( frame, deviceID, sensorID, flags, clamp );

				if( msg )
				{
					std::lock_guard<std::mutex> lock( _writeMsgMutex );

					ComMsg cpy = *msg;
					cpy.data = new unsigned char[msg->hdr.hdr.dataBytes];
					memcpy( cpy.data, msg->data, msg->hdr.hdr.dataBytes );

					_writeMsgQueue.push_back( cpy );

					while( _writeMsgQueue.size() > _maxQueueSize )
					{
						safeDeleteArray( _writeMsgQueue.front().data );
						_writeMsgQueue.pop_front();
					}

					return true;
				}

				return false;
			}

#ifdef __SUPPORT_GUI
			bool drawUI()
			{
				ImGui::Text( _portName.c_str() );
				ImGui::Text( "baud rate: %u", _baudRate );

				if( _port.isOpen() )
				{
					ImGui::Text( _isStarted ? "started" : "STOPPED" );
					ImGui::Text( _isSynced ? "synced" : "NOT SYNCED" );
					ImGui::Text( "queued (I/O): %d/%d", _readMsgQueue.size(), _writeMsgQueue.size() );

					{
						std::lock_guard<std::mutex> lock( _readMsgMutex );

						if( ImGui::TreeNode( "senders", "%d senders", _activeSenders.size() ) )
						{
							for( auto& it : _activeSenders )
							{
								uint16_t deviceID = ( it >> 8 ) & 0xff;
								uint16_t sensorID = ( it ) & 0xff;

								ImGui::Text( "  dID: %d, sID: %d", deviceID, sensorID );
							}

							ImGui::TreePop();
						}

						_activeSenders.clear();
					}
				}
				else
					ImGui::Text( "offline" );
				return true;
			}
#endif

			const std::string &getPortName() const { return _portName; }
			unsigned int getBaudRate() const { return _baudRate; }
			unsigned int getMaxQueueSize() const { return _maxQueueSize; }

			static bool init()
			{
				return SerialPort::init();
			}

			static bool isInitialized()
			{
				return SerialPort::isInitialized();
			}

			static void rescan()
			{
				SerialPort::rescan();
			}

			static void enumerate()
			{
				SerialPort::enumerate();
			}

			static const std::vector<std::string> &getKnownPorts() { return SerialPort::getKnownPorts(); }
			static const std::vector<std::string> &getKnownPortsNames() { return SerialPort::getKnownPortNames(); }
		};
	}



	bool COMInterface::init()
	{
		return Internal::COMInterfaceImpl::init();
	}

	bool COMInterface::isInitialized()
	{
		return Internal::COMInterfaceImpl::isInitialized();
	}

	void COMInterface::rescan()
	{
		Internal::COMInterfaceImpl::rescan();
	}

	void COMInterface::enumerate()
	{
		Internal::COMInterfaceImpl::enumerate();
	}




	COMInterface::COMInterface() :
		DataInterface(),
		_ioMode( IOMode::IOM_INPUT ),
		_clamp( false ),
		_normalize( true ),
		_dataType( MF_DATATYPE_FLOAT ),
		_portNr( 0 ),
		_impl( nullptr ),
		_baud( COM_SOURCE_BAUD_DEFAULT ),
		_queueSize( SOURCE_MAX_QUEUE_SIZE ),
		_dropdownSelected( ~0x00 )
	{}

	COMInterface::~COMInterface()
	{
		close();
	}

	bool COMInterface::run( const std::string &portName, unsigned int baud, unsigned int queueSize )
	{
		close();

		if( _ioMode == IOM_NONE )
			std::cerr << "<warning> no point in running COM interface -- select at least one of read or write mode" << std::endl;

		_impl = new Internal::COMInterfaceImpl( portName, baud, queueSize );

		if( !_impl->run( _ioMode ) )
		{
			std::cerr << "<error> running COM at " << portName << " with baud rate " << baud << " failed" << std::endl;
			safeDelete( _impl );
		}

		return false;
	}

	void COMInterface::close()
	{
		if( _impl )
		{
			_impl->close();
			safeDelete( _impl );
		}
	}

	bool COMInterface::doesWant( const SampleFrameContainer *sfc ) const
	{
		return ( _ioMode & IOM_OUTPUT );
	}

	void COMInterface::fetchFrames( std::vector<SampleFrameContainer> &frames )
	{
		if( _impl )
			_impl->fetchFrames( frames );
	}

	bool COMInterface::queueFrame( const SampleFrameContainer& sfc )
	{
		if( _impl )
		{
			MsgFlags flags = (MsgFlags) ( MF_NONE
				| ( _normalize ? MF_NONE : MF_NORMALIZED )
				| _dataType
				| MF_ENC_UNCOMPRESSED );
			if( _impl->write( sfc.frame, sfc.deviceID, sfc.sensorID, flags, _clamp ) )
				return true;
			std::cerr << "<error> failed to insert frame from sink" << std::endl;
		}
		return false;
	}

	std::string COMInterface::getDesc() const
	{
		if( _impl )
			return _impl->getDesc();
		return "<empty>";
	}

#ifdef __SUPPORT_GUI
	bool COMInterface::drawUI()
	{
		auto& devices = Internal::COMInterfaceImpl::getKnownPortsNames();

		{
			ScopedImGuiDisable disable( _impl != nullptr );

			std::vector<std::string> items;
			for( auto &it : devices )
				items.push_back( it );

			if( !items.size() )
				items.push_back( "<none>" );

			if( _dropdownSelected >= items.size() )
				_dropdownSelected = items.size() - 1;

			const char *currentItem = items[_dropdownSelected].c_str();

			if( ImGui::BeginCombo( "##combo", currentItem, ImGuiComboFlags_None ) )
			{
				for( int i = 0; i < items.size(); i++ )
				{
					bool isSelected = ( i == _dropdownSelected );
					if( ImGui::Selectable( items[i].c_str(), isSelected ) )
						_dropdownSelected = i;
					if( isSelected )
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if( ImGui::Button( "rescan" ) )
				_impl->rescan();

			int baud = _baud;
			if( ImGui::InputInt( "baud rate", &baud ) )
				_baud = baud;

			int s = _queueSize;
			if( ImGui::InputInt( "max queue size", &s ) )
				_queueSize = s;

			bool b = _ioMode & IOM_INPUT;
			if( ImGui::Checkbox( "do read", &b ) )
				_ioMode = (IOMode) ( ( _ioMode & ~IOM_INPUT ) | ( b ? IOM_INPUT : 0 ) );

			b = _ioMode & IOM_OUTPUT;
			if( ImGui::Checkbox( "do write", &b ) )
				_ioMode = (IOMode) ( ( _ioMode & ~IOM_OUTPUT ) | ( b ? IOM_OUTPUT : 0 ) );

			if( _ioMode & IOM_OUTPUT )
			{
				const char* name[] = { "byte", "short", "long", "float" };
				int t = _dataType >> 8;
				for( int i = 0; i < 4; i++ )
					ImGui::RadioButton( name[i], &t, i );
				_dataType = (MsgFlags) ( t << 8 );

				ImGui::Checkbox( "normalize", &_normalize );
				if( _normalize )
					ImGui::Checkbox( "clamp", &_clamp );

			}
		}

		if( _impl )
		{
			if( ImGui::Button( "close" ) )
				close();
			else
			{
				ImGui::Separator();

				_impl->drawUI();
			}
		}
		else
		{
			if( ImGui::Button( "open" ) )
			{
				if( _dropdownSelected < devices.size() )
				{
					std::string portName( Internal::COMInterfaceImpl::getKnownPorts()[_dropdownSelected] );

					_portNr = getPortNr( portName );
					run( portName, _baud, _queueSize );
				}
			}
		}


		return true;
	}
#endif

	bool COMInterface::loadFromJSON( const nlohmann::json &j )
	{
		if( !DataInterface::loadFromJSON( j ) )
			return false;

		safeDelete( _impl );

		try
		{
			unsigned int ioMode = 0;
			if( load<unsigned int>( j, "ioMode", ioMode ) )
				_ioMode = (IOMode) ioMode;

			load<int>( j, "dataType", _dataType );

			load<bool>( j, "normalize", _normalize );
			load<bool>( j, "clamp", _clamp );

			bool opened = false;
			load<bool>( j, "opened", opened );

			bool configComplete = true;
			if( !load<unsigned int>( j, "maxQueueSize", _queueSize ) )
				configComplete = false;

			if( !load<unsigned int>( j, "baudRate", _baud ) )
				configComplete = false;

			std::string portName;
			if( load<std::string>( j, "portName", portName ) )
			{
				_portNr = getPortNr( portName );

				auto &devices = Internal::COMInterfaceImpl::getKnownPorts();
				_dropdownSelected = ~0x00;
				for( int i = 0; i < devices.size(); i++ )
					if( getPortNr( devices[i] ) == _portNr )
					{
						_dropdownSelected = i;
						break;
					}
			}
			else
				configComplete = false;

			if( opened && configComplete )
				run( portName, _baud, _queueSize );
		}
		catch( std::exception &e )
		{
			std::cerr << "<error> loading COM interface props failed: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	void COMInterface::saveToJSON( nlohmann::json &j ) const
	{
		DataInterface::saveToJSON( j );

		save( j, "ioMode", (unsigned int) _ioMode );

		save( j, "dataType", _dataType );

		save( j, "normalize", _normalize );
		save( j, "clamp", _clamp );

		save( j, "opened", ( _impl ? true : false ) );
		if( _impl )
		{
			save( j, "portName", _impl->getPortName() );
			save( j, "baudRate", _impl->getBaudRate() );
			save( j, "maxQueueSize", _impl->getMaxQueueSize() );
		}
		else
		{
			auto &devices = Internal::COMInterfaceImpl::getKnownPorts();
			if( _dropdownSelected < devices.size() )
				save( j, "portName", devices[_dropdownSelected] );

			save( j, "baudRate", _baud );
			save( j, "maxQueueSize", _queueSize );
		}
	}
}