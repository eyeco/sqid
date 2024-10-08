/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "comSource.h"

#include "internal/fwProps.h"

#include "serialMsg.h"
#include "../ringBuffer.h"

#include <fileIO/json.h>


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
		class COMSourceImpl : public IAbstractWriter
		{
		private:
			enum ProtocolState
			{
				PS_SYNCING,
				PS_HEADER,
				PS_PAYLOAD,

				PS_COUNT
			};

			FWProps *_fwProps;

			bool _isSynced;
			ProtocolState _currentState;
			unsigned int _expectedBytes;

			unsigned int _maxQueueSize;
			std::list<ComMsg> _msgQueue;
			std::mutex _msgMutex;

			std::set<uint16_t> _activeSenders;

			std::map<std::string,std::string> _propMap;
			std::mutex _propMutex;

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

			bool tryConnect()
			{
				this->closeCOM();

				if( !_port.open( _portName, _baudRate ) )
				{
					std::cerr << "failed to connect com source" << std::endl;

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

					//unsigned char b = 0;
					std::vector<unsigned char> b( _readBufferSize );

					try
					{
						while( _keepRunning )
						{
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
											std::cerr << "<error> serial protocol " << _hdrEx.hdr.ver << " not supported (requiring PV" << PV_2 << "), stopping reading from source..." << std::endl;
											
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

											ComMsg msg;
											msg.hdr = _hdrEx;
											msg.data = new unsigned char[_expectedBytes];
											memcpy( msg.data, &_dataBuffer[0], _expectedBytes );

											if( msg.hdr.hdr.type == MT_PROPERTY )
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
											else if( msg.hdr.hdr.type == MT_FWPROPS_DESC )
											{
												if( _fwProps )
													_fwProps->onDesc( reinterpret_cast<const unsigned char*>( msg.data ), msg.hdr.hdr.dataBytes );
												safeDeleteArray( msg.data );
											}
											else if( msg.hdr.hdr.type == MT_FWPROPS_STATUS )
											{
												if( _fwProps )
													_fwProps->onStatus( reinterpret_cast<const unsigned char*>( msg.data ), msg.hdr.hdr.dataBytes );
												safeDeleteArray( msg.data );
											}
											else if( msg.hdr.hdr.type == MT_FWPROPS_ACK )
											{
												if( _fwProps )
													_fwProps->onAck( reinterpret_cast<const unsigned char*>( msg.data ), msg.hdr.hdr.dataBytes );
												safeDeleteArray( msg.data );
											}
											else
											{
												std::lock_guard<std::mutex> lock( _msgMutex );

												//TODO: there was a mem-leak (32 bytes) reported around here somewhere, which I never had before, revisit this
												_msgQueue.push_back( msg );

												_activeSenders.insert( msg.hdr.hdr.deviceID << 8 | msg.hdr.hdr.sensorID );

												while( _msgQueue.size() > _maxQueueSize )
												{
													safeDeleteArray( _msgQueue.front().data );
													_msgQueue.pop_front();
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

		public:
			COMSourceImpl( const std::string &portName, unsigned int baudRate, unsigned int maxQueueSize ) :
				_fwProps( new FWProps( this ) ),
				_isSynced( false ),
				_currentState( PS_COUNT ),
				_maxQueueSize( maxQueueSize ),
				_expectedBytes( 0 ),
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

			~COMSourceImpl()
			{
				this->close();

				safeDelete( _fwProps );
			}

			bool run()
			{
				//TODO: this query is not *entirely* threadsafe, but it should do...
				if( _isStarted )
					return false;
				_isStarted = true;

				if( _port.isOpen() )	//already running
					return false;

				_keepRunning = true;
				_comListenerThread = new std::thread( &COMSourceImpl::comListen, this );

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
					std::lock_guard<std::mutex> lock( _msgMutex );

					for( auto it = _msgQueue.begin(); it != _msgQueue.end(); ++it )
						safeDeleteArray( it->data );
					_msgQueue.clear();
				}

				_activeSenders.clear();
			}

			std::string getDesc() const
			{
				return _parser.getDesc();
			}

			void fetchFrames( std::vector<SampleFrameContainer> &frames )
			{
				if( frames.size() )
					std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory!" << std::endl;

				if( _msgQueue.size() )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					while( _msgQueue.size() )
					{
						ComMsg &msg = _msgQueue.front();
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

						_msgQueue.pop_front();
					}
				}
			}

			virtual bool write( const unsigned char *data, size_t size )
			{
				if( !_port.isOpen() )
					return false;

				return ( _port.write( data, size ) == size );
			}

#ifdef __SUPPORT_GUI
			bool drawUI()
			{
				ImGui::Text( _portName.c_str() );
				ImGui::Text( "baud rate: %u", _baudRate );

				if( _port.isOpen() )
				{
					if( _fwProps )
						_fwProps->drawUI();

					ImGui::Text( _isStarted ? "started" : "STOPPED" );
					ImGui::Text( _isSynced ? "synced" : "NOT SYNCED" );
					ImGui::Text( "queued: %d", _msgQueue.size() );

					if( ImGui::TreeNode( "senders", "%d senders", _activeSenders.size() ) )
					{
						for( auto &it : _activeSenders )
						{
							char deviceID = ( it >> 8 ) & 0xff;
							char sensorID = ( it ) & 0xff;

							ImGui::Text( "  dID: %d, sID: %d", deviceID, sensorID );
						}

						ImGui::TreePop();
					}

					_activeSenders.clear();
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
		};
	}



	bool COMSource::init()
	{
		return Internal::COMSourceImpl::init();
	}

	bool COMSource::isInitialized()
	{
		return Internal::COMSourceImpl::isInitialized();
	}

	void COMSource::rescan()
	{
		Internal::COMSourceImpl::rescan();
	}

	void COMSource::enumerate()
	{
		Internal::COMSourceImpl::enumerate();
	}




	COMSource::COMSource() :
		DataSource(),
		_portNr( 0 ),
		_impl( nullptr ),
		_baud( COM_SOURCE_BAUD_DEFAULT ),
		_queueSize( SOURCE_MAX_QUEUE_SIZE ),
		_dropdownSelected( ~0x00 )
	{}

	COMSource::~COMSource()
	{
		close();
	}

	bool COMSource::run( const std::string &portName, unsigned int baud, unsigned int queueSize )
	{
		_impl = new Internal::COMSourceImpl( portName, baud, queueSize );

		if( !_impl->run() )
		{
			std::cerr << "<error> running COM at " << portName << " with baud rate " << baud << " failed" << std::endl;
			safeDelete( _impl );
		}

		if( _impl )
			return _impl->run();
		return false;
	}

	void COMSource::close()
	{
		if( _impl )
		{
			_impl->close();
			safeDelete( _impl );
		}
	}

	void COMSource::fetchFrames( std::vector<SampleFrameContainer> &frames )
	{
		if( _impl )
			_impl->fetchFrames( frames );
	}

	std::string COMSource::getDesc() const
	{
		if( _impl )
			return _impl->getDesc();
		return "<empty>";
	}

	bool COMSource::write( const unsigned char *data, size_t size )
	{
		if( _impl )
			return _impl->write( data, size );
		return false;
	}

	bool COMSource::write( const std::string &str )
	{
		//TODO: be careful here with wstring
		if( _impl )
			return _impl->write( (unsigned char*)str.c_str(), str.size() + 1 );
		return false;
	}

	bool COMSource::write( const std::vector<unsigned char> &data )
	{
		if( _impl )
			return _impl->write( &data[0], data.size() );
		return false;
	}

#ifdef __SUPPORT_GUI
	bool COMSource::drawUI()
	{
		//std::stringstream sstr;
		//sstr << interfaceToString( getDeviceInterface() ) << "##" << guidToString( _objectID );
		//std::string desc = sstr.str();

		if( _impl )
		{
			_impl->drawUI();

			if( ImGui::Button( "close" ) )
				close();
		}
		else
		{
			auto &devices = Internal::COMSourceImpl::getKnownPorts();
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

			if( ImGui::Button( "open" ) )
			{
				if( _dropdownSelected < devices.size() )
				{
					std::string portName( devices[_dropdownSelected] );

					_portNr = getPortNr( portName );
					run( portName, _baud, _queueSize );
				}
			}
		}

		return true;
	}
#endif

	bool COMSource::loadFromJSON( const nlohmann::json &j )
	{
		if( !DataSource::loadFromJSON( j ) )
			return false;

		safeDelete( _impl );

		try
		{
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

				auto &devices = Internal::COMSourceImpl::getKnownPorts();
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
			std::cerr << "<error> loading COM source props failed: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	void COMSource::saveToJSON( nlohmann::json &j ) const
	{
		DataSource::saveToJSON( j );

		save( j, "opened", ( _impl ? true : false ) );
		if( _impl )
		{
			save( j, "portName", _impl->getPortName() );
			save( j, "baudRate", _impl->getBaudRate() );
			save( j, "maxQueueSize", _impl->getMaxQueueSize() );
		}
		else
		{
			auto &devices = Internal::COMSourceImpl::getKnownPorts();
			if( _dropdownSelected < devices.size() )
				save( j, "portName", devices[_dropdownSelected] );

			save( j, "baudRate", _baud );
			save( j, "maxQueueSize", _queueSize );
		}
	}
}