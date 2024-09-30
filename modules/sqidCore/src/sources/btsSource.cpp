/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "btsSource.h"

#ifdef __RFCOMM_SUPPORT

#include "internal/fwProps.h"

#include "serialMsg.h"
#include "../ringBuffer.h"

#include <fileIO/json.h>

#include <commonImGui.h>


#include <map>
#include <list>
#include <mutex>
#include <thread>
#include <iostream>


#include <DeviceINQ.h>
#include <BTSerialPortBinding.h>
#include <BluetoothException.h>

#pragma comment( lib, "bluetoothserialport.lib" )
#pragma comment( lib, "Bthprops.lib" )

namespace sqid
{
	namespace Internal
	{
		std::string formatDate( const char *format, time_t time )
		{
			if( time <= 0 )
				return "--";

			char buffer[256] = { 0 };
			tm *timeinfo = localtime( &time );

			if( timeinfo )
				strftime( buffer, sizeof( buffer ), format, timeinfo );

			return buffer;
		}

		struct BTSDevice
		{
			std::string address;
			int channel;

			std::string name;
			std::time_t lastSeen;
			std::time_t lastUsed;

			bool connected;
			bool remembered;
			bool authenticated;

			DeviceClass deviceClass;
			DeviceClass majorDeviceClass;
			ServiceClass serviceClass;
		};

		//TODO: remove duplicate code (this class is quite similar to COMSourceImpl) by moving all 
		// the protocol-specific stuff (parsing, etc.) to a shared class
		class BTSSourceImpl : public IAbstractWriter
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

			unsigned int _maxMsgQueueSize;
			std::list<ComMsg> _msgQueue;
			std::mutex _msgMutex;

			std::map<std::string,std::string> _propMap;
			std::mutex _propMutex;

			RingBuffer<unsigned char> _buffer;

			ComMsgHdrEx _hdrEx;
			std::vector<unsigned char> _dataBuffer;

			ComMsgParser _parser;

			bool _isStarted;

			bool _keepRunning;
			bool _tryReconnect;

			unsigned int _id;

			std::string _name;
			std::string _address;

			int _btsChannel;
			BTSerialPortBinding *_bts;

			int _reconnectDelay;

			std::thread *_btsListenerThread;

			static bool initialized;
			static std::vector<BTSDevice> knownDevices;

			bool tryConnect()
			{
				this->closeBTS();

				std::cout << "opening Bluetooth Serial #" << _id << " \"" << _name << "\" @ " << _address << std::endl;

				_bts = BTSerialPortBinding::Create( _address, _btsChannel );

				if( !_bts )
				{
					std::cerr << "failed to open Bluetooth Serial #" << _id << " \"" << _name << "\" @ " << _address << std::endl;

					this->closeBTS();
					return false;
				}

				_bts->Connect();

				_isSynced = false;
				_currentState = PS_SYNCING;
				_expectedBytes = SYNC_BYTES;

				return true;
			}

			void btsListen()
			{
				std::vector<char> buffer( 1024 );

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

					char b = 0;

					try
					{
						while( _keepRunning )
						{
							bool available = false;
							try
							{
								available = _bts->IsDataAvailable();
							}
							catch( BluetoothException &be )
							{
								std::cerr << "<error> caught BT exception: " << be.what() << std::endl;

								_keepRunning = false;
								_tryReconnect = false;

								break;
							}

							if( !available )
								std::this_thread::sleep_for( std::chrono::microseconds( 1 ) );
							else
							{
								bytesRead = _bts->Read( &b, 1 );

								if( !bytesRead )
								{
									//std::cout << "no bytes read" << std::endl;
									continue;
								}

								_buffer.add( b );
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
												std::cout << "synced BTE #" << _id << " \"" << _name << "\" @ " << _address << std::endl;
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

												_msgQueue.push_back( msg );

												while( _msgQueue.size() > _maxMsgQueueSize )
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
						std::cerr << "caught exception in btsListen: " << e.what() << std::endl;
					}
				}
			}

			void closeBTS()
			{
				_currentState = PS_COUNT;
				if( _bts )
				{
					_bts->Close();
					safeDelete( _bts );
				}
			}

		public:
			BTSSourceImpl( const std::string &name, const std::string &address, unsigned int id, unsigned int maxMsgQueueSize ) :
				_fwProps( new FWProps( this ) ),
				_isSynced( false ),
				_currentState( PS_COUNT ),
				_maxMsgQueueSize( maxMsgQueueSize ),
				_expectedBytes( 0 ),
				_dataBuffer( 1024 ),
				_isStarted( false ),
				_keepRunning( false ),
				_tryReconnect( true ),
				_id( id ),
				_name( name ),
				_address( address ),
				_btsChannel( 1 ), //must be 1 (but read from DeviceINQ, just to be sure; TODO!)
				_bts( nullptr ),
				_reconnectDelay( 5 ),
				_btsListenerThread( nullptr )
			{}

			~BTSSourceImpl()
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

				bool found = false;
				for( auto &d : knownDevices )
				{
					if( !d.name.compare( _name ) )
					{
						if( found ) //already found earlier
						{
							std::cerr << "<warning> multiple Bluetooth Serial devices with requested name \"" << _name << "\" are paired -- chosing first one with address " << _address << std::endl;
							continue;
						}
						if( !_address.length() ) //address not specified -- using first in list with matching name
						{
							found = true;
							_address = d.address;
							std::cout << "found Bluetooth Serial device with requested name \"" << _name << "\". address is " << _address << std::endl;
						}
					}
					if( _address.length() && !d.address.compare( _address ) )
					{
						found = true;

						if( d.name.compare( _name ) )
						{
							std::cerr << "<warning> device with requested address " << _address << " is named \"" << d.name << "\" instead of requested \"" << _name << "\" -- adopting real name" << std::endl;
							_name = d.name;
						}
						else
							std::cout << "found Bluetooth Serial device with requested name \"" << _name << "\" and address " << _address << std::endl;
					}
				}

				if( !found )
				{
					if( _address.length() )
						std::cerr << "<error> requested Bluetooth Serial device \"" << _name << "\" with address " << _address << " not found" << std::endl;
					else
						std::cerr << "<error> requested Bluetooth Serial device \"" << _name << "\" not found" << std::endl;
					return false;
				}

				_keepRunning = true;
				_btsListenerThread = new std::thread( &BTSSourceImpl::btsListen, this );

				return true;
			}

			void close()
			{
				if( _btsListenerThread )
				{
					//NOTE: don't call closeBTS here, as it would delete the bts instance and cause
					// a segfault in the btsListen, but do trigger a disconnect, so bts::Read returns
					if( _bts )
						_bts->Close();

					_keepRunning = false;
					_tryReconnect = false;
					_btsListenerThread->join();

					safeDelete( _btsListenerThread );
				}

				this->closeBTS();

				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					for( auto it = _msgQueue.begin(); it != _msgQueue.end(); ++it )
						safeDeleteArray( it->data );
					_msgQueue.clear();
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

				if( _msgQueue.size() )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					while( _msgQueue.size() )
					{
						ComMsg &msg = _msgQueue.front();
						SampleFrame *frame = nullptr;

						switch( msg.hdr.hdr.type )
						{
						case MT_SINGLE_VALUE:
							frame = _parser.createFrameFromSingleValue( msg.hdr.hdr, msg.data );
							break;
						case MT_ARRAY:
							frame = _parser.createFrameFromArray( msg.hdr.hdr, msg.data );
							break;
						case MT_MATRIX:
							frame = _parser.createFrameFromMatrix( msg.hdr.hdr, msg.data );
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
				if( !_bts )
					return false;

				_bts->Write( reinterpret_cast<const char*>( data ), size );

				return true;
			}

#ifdef __SUPPORT_GUI
			void drawUI()
			{
				ImGui::Text( _name.c_str() );
				ImGui::Text( _address.c_str() );

				if( _bts )
				{
					if( _fwProps )
						_fwProps->drawUI();
				}
				else
					ImGui::Text( "offline" );
			}
#endif

			static bool init()
			{
				if( !initialized )
				{
					rescan();
					initialized = true;
				}

				return initialized;
			}

			static bool isInitialized()
			{
				return initialized;
			}

			static void rescan()
			{
				knownDevices.clear();

				std::cout << "scanning for Bluetooth Serial devices..." << std::endl;

				try
				{
					std::unique_ptr<DeviceINQ> inq( DeviceINQ::Create() );
					std::vector<device> devices = inq->Inquire();

					for( const auto &d : devices )
					{
						BTSDevice btsd;

						btsd.address = d.address;
						try
						{
							btsd.channel = inq->SdpSearch( d.address );
						}
						catch( BluetoothException &e )
						{
							std::cerr << "<error> caught exception doing SDP search for \"" << d.address << "(" << d.name << "): " << e.what() << std::endl;
							continue;
						}

						btsd.name = d.name;
						btsd.lastSeen = d.lastSeen;
						btsd.lastUsed = d.lastUsed;

						btsd.connected = d.connected;
						btsd.remembered = d.remembered;
						btsd.authenticated = d.authenticated;

						btsd.deviceClass = d.deviceClass;
						btsd.majorDeviceClass = d.majorDeviceClass;
						btsd.serviceClass = d.serviceClass;

						knownDevices.push_back( btsd );
					}

					std::cout << "found " << knownDevices.size() << " Bluetooth Serial device(s)" << std::endl;
				}
				catch( BluetoothException &e )
				{
					std::cerr << "<error> caught exception scanning for devices: " << e.what() << std::endl;
				}
			}

			static void enumerate()
			{
				int cntr = 0;
				for( const auto& d : knownDevices )
				{
					std::cout << "  device #" << cntr++ << std::endl;
					std::cout << "    name: " << d.name << std::endl;
					std::cout << "    address: " << d.address << std::endl;
					std::cout << "    class: " << GetDeviceClassString( d.deviceClass ) << std::endl;
					std::cout << "    major class: " << GetDeviceClassString( d.majorDeviceClass ) << std::endl;
					std::cout << "    service class: " << GetServiceClassString( d.serviceClass ) << std::endl;
					std::cout << "    last seen: " << formatDate( "%c", d.lastSeen ) << std::endl;
					std::cout << "    last used: " << formatDate( "%c", d.lastUsed ) << std::endl;
					std::cout << "    channel ID: " << d.channel << std::endl;
				}
			}

			static const std::vector<BTSDevice> &getKnownDevices() { return knownDevices; }
		};

		bool BTSSourceImpl::initialized = false;
		std::vector<BTSDevice> BTSSourceImpl::knownDevices;
	}

	unsigned int BTSSource::idCntr = 0;

	bool BTSSource::init()
	{
		return Internal::BTSSourceImpl::init();
	}

	bool BTSSource::isInitialized()
	{
		return Internal::BTSSourceImpl::isInitialized();
	}

	void BTSSource::rescan()
	{
		Internal::BTSSourceImpl::rescan();
	}

	void BTSSource::enumerate()
	{
		Internal::BTSSourceImpl::enumerate();
	}

	BTSSource::BTSSource() :
		DataSource(),
		_id( idCntr++ ),
		_name( "" ),
		_address( "" ),
		_impl( nullptr ),
		_dropdownSelected( ~0x00 )
	{}

	BTSSource::~BTSSource()
	{
		close();
	}

	bool BTSSource::run( const std::string &name, const std::string &address )
	{
		close();

		_name = name;
		_address = address;

		_impl = new Internal::BTSSourceImpl( name, address, _id, SOURCE_MAX_QUEUE_SIZE );
		if( !_impl->run() )
		{
			safeDelete( _impl );
			return false;
		}

		return true;
	}

	void BTSSource::close()
	{
		if( _impl )
		{
			_impl->close();
			safeDelete( _impl );
		}
	}

	std::string BTSSource::getDesc() const
	{
		return _impl->getDesc();
	}

	void BTSSource::fetchFrames( std::vector<SampleFrameContainer> &frames )
	{
		if( _impl )
			_impl->fetchFrames( frames );
	}

	bool BTSSource::write( const unsigned char *data, size_t size )
	{
		if( _impl )
			return _impl->write( data, size );
		return false;
	}

	bool BTSSource::write( const std::string &str )
	{
		//TODO: be careful here with wstring
		if( _impl )
			return _impl->write( (unsigned char*)str.c_str(), str.size() + 1 );
		return false;
	}

	bool BTSSource::write( const std::vector<unsigned char> &data )
	{
		return _impl->write( &data[0], data.size() );
	}

#ifdef __SUPPORT_GUI
	bool BTSSource::drawUI()
	{
		std::stringstream sstr;
		sstr << interfaceToString( getDeviceInterface() ) << getDevicePort();
		std::string desc = sstr.str();

		if( _impl )
		{
			_impl->drawUI();

			if( ImGui::Button( "close" ) )
				close();
		}
		else
		{
			auto &devices = Internal::BTSSourceImpl::getKnownDevices();
			std::vector<std::string> items;
			for( auto &it : devices )
				items.push_back( it.name + " [" + it.address + "]" );

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

			if( ImGui::Button( "open" ) )
			{
				if( _dropdownSelected < devices.size() )
					run( devices[_dropdownSelected].name, devices[_dropdownSelected].address );
			}
		}

		return true;
	}
#endif


	bool BTSSource::loadFromJSON( const nlohmann::json &j )
	{
		if( !DataSource::loadFromJSON( j ) )
			return false;

		safeDelete( _impl );

		load<unsigned int>( j, "btsID", _id );
		load<std::string>( j, "btsName", _name );
		load<std::string>( j, "btsAddress", _address );

		bool started = false;
		load<bool>( j, "started", started );

		if( idCntr <= _id )
			idCntr = _id + 1;

		_dropdownSelected = ~0x00;
		auto &devices = Internal::BTSSourceImpl::getKnownDevices();
		for( int i = 0; i < devices.size(); i++ )
			if( !devices[i].name.compare( _name ) && !devices[i].address.compare( _address ) )
				_dropdownSelected = i;
		if( _dropdownSelected == ~0x00 )
		{
			std::cerr << "<warning> BTS device " << _name << " with address " << _address << " not found -- resetting" << std::endl;

			_name = "";
			_address = "";
			started = false;
		}
		
		if( started )
			run( _name, _address );

		return true;
	}

	void BTSSource::saveToJSON( nlohmann::json &j ) const
	{
		DataSource::saveToJSON( j );

		save( j, "btsID", _id );
		save( j, "btsName", _name );
		save( j, "btsAddress", _address );

		save( j, "started", ( _impl ? true : false ) );
	}
}
#endif // __RFCOMM_SUPPORT