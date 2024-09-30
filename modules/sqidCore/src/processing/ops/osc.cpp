/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "osc.h"

#include "../sceneGraph.h"

//#include <drawing/frameDrawer.h>

#include <fileIO/json.h>

//#include <processing/pin.h>

#include <commonImGui.h>

#include <opencv2/imgproc.hpp>
//#include <opencv2/imgproc/types_c.h>

#include <lo/lo.h>

#include <thread>
#include <string.h>

#ifdef _WIN32
#define strnicmp _strnicmp
#else
#define strnicmp strncasecmp
#endif

namespace sqid
{
	namespace OSC
	{
		namespace Internal
		{
			static void loErrorHandler( int num, const char *msg, const char *where )
			{
				if( msg && where )
					std::cerr << "<error> liblo server error " << num << " in path " << where << ": " << msg << std::endl;
				else if( msg )
					std::cerr << "<error> liblo server error " << num << ": " << msg << std::endl;
				else if( where )
					std::cerr << "<error> liblo server error " << num << " in path " << where << std::endl;
				else
					std::cerr << "<error> liblo server error " << num << std::endl;
			}

			struct OSCMsg
			{
				OSCMsg( const std::string &senderIP, const std::string &addressPattern, unsigned char deviceID, unsigned char sensorID, unsigned short width, unsigned short height, unsigned short depth, const float *values, uint32_t timestamp ) :
					senderIP( senderIP ), addressPattern( addressPattern ),
					deviceID( deviceID ), sensorID( sensorID ),
					width( width ), height( height ), depth( depth ),
					timestamp( timestamp ),
					values( width * height * depth )
				{
					if( this->values.size() )
						memcpy( &this->values[0], values, this->values.size() * sizeof( float ) );
				}

				std::string senderIP;
				std::string addressPattern;

				unsigned char deviceID;
				unsigned char sensorID;

				unsigned short width;
				unsigned short height;
				unsigned short depth;

				uint32_t timestamp;

				std::vector<float> values;
			};

			class OSCSourceImpl
			{
			private:
				std::list<OSCMsg> msgQueue;
				std::mutex msgMutex;

				lo_server server;

				std::thread *oscListenerThread;

				std::string filter;

				bool keepRunning;

				void oscListen()
				{
					while( keepRunning )
					{
						if( server )
							lo_server_recv_noblock( server, 1 );
						else
							break;
					}
				}

				static int messageHandler( const char *path, const char *types, lo_arg ** argv, int argc, lo_message msg, void *userData )
				{
					return reinterpret_cast<OSCSourceImpl*>( userData )->messageHandler( path, types, argv, argc, msg );
				}

			protected:

				int messageHandler( const char *path, const char *types, lo_arg ** argv, int argc, lo_message msg )
				{
					try
					{
						lo_address srcAddr = lo_message_get_source( msg );
						const char *srcHost = lo_address_get_hostname( srcAddr );
						//const char *srcPort = lo_address_get_port( srcAddr );

						if( argc != 7 )
						{
							std::cerr << "<error> unexpected number of OSC arguments: " << argc << std::endl;
							return -1;
						}

						//types = 0x0000020a570086f1 "iiiiiib"

						if( types[0] != 'i' && types[0] != 'c' )
							std::cerr << "<warning> invalid deviceID type: '" << types[0] << "'" << std::endl;
						if( types[1] != 'i' && types[1] != 'c' )
							std::cerr << "<warning> invalid sensorID type: '" << types[1] << "'" << std::endl;
						if( types[2] != 'i' || types[3] != 'i' || types[4] != 'i' )
							std::cerr << "<warning> invalid size types: '" << types[2] << types[3] << types[4] << "'" << std::endl;
						if( types[5] != 'i' )
							std::cerr << "<warning> invalid timestamp type: '" << types[5] << "'" << std::endl;
						if( types[6] != 'b' )
						{
							std::cerr << "<error> invalid data block type: '" << types[6] << "' (should be blob)" << std::endl;
							return -1;
						}

						//NOTE: bytes/chars are non-OSC-standard, so use int32
						// also, 'c' extensions are using 32b ints anyways, so you won't save any bandwidth
						unsigned char deviceID = (unsigned char) ( argv[0]->i32 );
						unsigned char sensorID = (unsigned char) ( argv[1]->i32 );

						unsigned short width = (unsigned short) ( argv[2]->i32 );
						unsigned short height = (unsigned short) ( argv[3]->i32 );
						unsigned short depth = (unsigned short) ( argv[4]->i32 );
						uint32_t ts = (uint32_t) ( argv[5]->i32 );

						const float *values = reinterpret_cast<float*>( &argv[6]->blob.data );
						unsigned int size = width * height * depth;
						if( argv[6]->blob.size != size * sizeof( float ) )
						{
							std::cerr << "<warning> OSC blob has unexpected size" << std::endl;
							return -1;
						}

						{
							std::lock_guard<std::mutex> lock( msgMutex );

							if( filter.size() && strnicmp( path, this->filter.c_str(), this->filter.size() ) )
								return 1;

							msgQueue.push_back( OSCMsg( std::string( srcHost ), std::string( path ), deviceID, sensorID, width, height, depth, values, ts ) );
						}
					}
					catch( std::exception &e )
					{
						std::cerr << "error while parsing message: " << path << ": " << e.what() << std::endl;

						return -1;
					}

					return 0;
				}

			public:
				explicit OSCSourceImpl() :
					server( nullptr ),
					oscListenerThread( nullptr ),
					filter( "" ),
					keepRunning( false )
				{}

				~OSCSourceImpl()
				{
					this->close();
				}

				bool run( const std::string &ip, unsigned short port, Protocol proto )
				{
					if( this->oscListenerThread || server )
						return false;

					switch( proto )
					{
					case P_UDP:
						std::cerr << "trying to open OSC on port " << port << " (UDP)" << std::endl;
						server = lo_server_new( toString( port ).c_str(), loErrorHandler );

						if( !server )
						{
							std::cerr << "<error> creating server failed" << std::endl;
							return false;
						}

						//NOTE: will do filtering on my own since i want it to be caseINsensitive (also there is a setter which will enable to change the filter at a later point)
						lo_server_add_method( server, NULL, NULL, OSCSourceImpl::messageHandler, this );

						break;
					case P_TCP:
						std::cerr << "trying to open OSC at " << ip << ":" << port << " (TCP)" << std::endl;
						server = lo_server_new_with_proto( toString( port ).c_str(), LO_TCP, loErrorHandler );

						if( !server )
						{
							std::cerr << "<error> creating server failed" << std::endl;
							return false;
						}

						//NOTE: will do filtering on my own since i want it to be caseINsensitive (also there is a setter which will enable to change the filter at a later point)
						lo_server_add_method( server, NULL, NULL, OSCSourceImpl::messageHandler, this );

						break;
					default:
						std::cerr << "<error> unknown protocol" << std::endl;
						return false;
					}

					keepRunning = true;

					this->oscListenerThread = new std::thread( &OSCSourceImpl::oscListen, this );

					return true;
				}

				void close()
				{
					keepRunning = false;
					if( this->oscListenerThread )
					{
						this->oscListenerThread->join();

						safeDelete( oscListenerThread );
					}
					if( server )
					{
						lo_server_free( server );
						server = nullptr;
					}
				}

				bool fetchFrames( std::vector<SampleFrame*> &frames )
				{
					if( frames.size() )
						std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory here!" << std::endl;

					if( msgQueue.size() )
					{
						std::lock_guard<std::mutex> lock( msgMutex );

						while( msgQueue.size() )
						{
							const OSCMsg &msg = msgQueue.front();

							//TODO: filter by dID and sID
							//msg.deviceID;
							//msg.sensorID;

							frames.push_back( new SampleFrame( msg.width, msg.height, &msg.values[0], msg.timestamp, msg.depth ) );

							msgQueue.pop_front();
						}

						return true;
					}

					return false;
				}

				const std::string &getFilter() const
				{
					return filter;
				}

				void setFilter( const std::string &filter )
				{
					std::lock_guard<std::mutex> lock( msgMutex );
					this->filter = filter;
				}
			};
		}

		DEFINE_OP_DESC( OSCIn, "oscIn", "/networking/deprecated",
			"DB1F2DE1-99DE-49DC-B723-F800733E40CB" );
		DEFINE_OP_DESC( OSCOut, "oscOut", "/networking",
			"BE71EF33-3562-40E6-BB34-91C5FBDDADB8" );





		OSCIn::OSCIn( const std::string &ip, unsigned short port ) :
			Op(),
			_connected( false ),
			_proto( P_UDP ),
			_ip( ip ),
			_port( port ),
			_inputBufferIP( 32 ), 
			_impl( new Internal::OSCSourceImpl() )
		{
		}

		OSCIn::~OSCIn()
		{
			safeDelete( _impl );
		}

		void OSCIn::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool OSCIn::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _connected ? "disconnect" : "connect" ) )
			{
				if( _connected )
					stop();
				else
					start();
			}

			{
				ScopedImGuiDisable disable( _connected );

				if( _impl )
				{
					int p = (int) _proto;
					for( int i = 0; i < FF_COUNT; i++ )
						ImGui::RadioButton( protocolToString( (Protocol) i ), &p, i );
					if( _proto != (Protocol) p )
						_proto = (Protocol) p;


					{
						ScopedImGuiDisable disableIP( _proto == P_UDP );

						if( ImGui::InputText( "ip", &_inputBufferIP[0], _inputBufferIP.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
						{
							bool valid = true;

							std::string str = trim( &_inputBufferIP[0] );
							std::vector<std::string> subs = split( str, '.' );
							if( subs.size() != 4 )
								valid = false;
							else for( auto it = subs.begin(); it != subs.end(); ++it )
								for( auto it2 = it->begin(); it2 != it->end(); ++it2 )
									if( *it2 < '0' || *it2 > '9' )
										valid = false;

							if( valid )
								_ip = str;
							else
								strcpy( &_inputBufferIP[0], _ip.c_str() );

							updateBuffers();
						}
					}


					int i = _port;
					if( ImGui::InputInt( "port", &i ) )
						_port = clamp<int>( i, 0x0000, 0xffff );

					char tempStr[256];
					strcpy( tempStr, _filter.c_str() );
					if( ImGui::InputText( "filter", tempStr, 256, ImGuiInputTextFlags_EnterReturnsTrue ) )
					{
						std::cout << "setting filter to " << tempStr << std::endl;
						_filter = std::string( tempStr );
						_impl->setFilter( _filter );
					}
				}
			}

			return true;
		}
#endif

		//TODO: when dealing with multiple inputs, this does not really make sense
		// think of a better way to do this
		bool OSCIn::process()
		{
			if( _impl && _connected )
			{
				std::vector<SampleFrame*> frames;

				_impl->fetchFrames( frames );

				if( frames.size() )
				{
					//TODO: buffer -- for now, we just use the most recent one
					SampleFrame *ret = frames.back();

					if( ret )
					{
						drawFrame( ret );

						pushOutput( "out", ret );
						ret = nullptr;
					}

					for( int i = 0; i < frames.size(); i++ )
						safeDelete( frames[i] );
					frames.clear();
				}
			}

			return false;
		}

		bool OSCIn::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			std::string str;
			if( load<std::string>( j, "proto", str ) )
				_proto = protocolFromString( str );
			load<std::string>( j, "ip", _ip );
			load<bool>( j, "connected", _connected );
			load<int>( j, "port", _port );
			load<std::string>( j, "filter", _filter );

			updateBuffers();

			if( _connected )
				start();

			return ret;
		}

		bool OSCIn::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "proto", protocolToString( _proto ) );
			save( j, "ip", _ip );
			save( j, "connected", _connected );
			save( j, "port", (int) _port );
			save( j, "filter", _filter );

			return ret;
		}

		void OSCIn::updateBuffers()
		{
			strncpy( &_inputBufferIP[0], _ip.c_str(), _ip.size() + 1 );
		}

		void OSCIn::start()
		{
			stop();

			_impl->setFilter( _filter );
			if( _impl->run( _ip, _port, _proto ) )
				_connected = true;
			else
			{
				std::cerr << "<error> failed to open OSC on port " << (int) _port << std::endl;
				stop();
			}
		}

		void OSCIn::stop()
		{
			if( _impl )
				_impl->close();

			_connected = false;
		}










		

		const char *outTypeToString( OSCOut::OutType type )
		{
			switch( type )
			{
			case OSCOut::OT_SAMPLEFRAME:
				return "sampleframe";
			case OSCOut::OT_NATIVE:
				return "native";
			case OSCOut::OT_NATIVE_CUSTOM:
				return "custom";
			}

			return "UNKNOWN";
		}

		OSCOut::OutType outTypeFromString( const char *s )
		{
			if( !s )
				return OSCOut::OT_COUNT;

			for( int i = 0; i < OSCOut::OT_COUNT; i++ )
				if( !_stricmp( s, outTypeToString( ( OSCOut::OutType ) i ) ) )
					return ( OSCOut::OutType ) i;

			return OSCOut::OT_COUNT;
		}

		OSCOut::OutType outTypeFromString( const std::string &s )
		{
			return outTypeFromString( s.c_str() );
		}

		const char *nativeTypeToString( OSCOut::NativeType nt )
		{
			switch( nt )
			{
			case OSCOut::NT_INT32:
				return "int32";
			case OSCOut::NT_FLOAT:
				return "float";
			//case OSCOut::NT_STRING:
			//	return "string";
			//case OSCOut::NT_BLOB:
			//	return "blob";
			case OSCOut::NT_INT64:
				return "int64";
			//case OSCOut::NT_TIMETAG:
			//	return "time";
			case OSCOut::NT_DOUBLE:
				return "double";
			//case OSCOut::NT_SYMBOL:
			//	return "symbol";
			case OSCOut::NT_CHAR:
				return "char";
			//case OSCOut::NT_MIDI:
			//	return "midi";
			//case OSCOut::NT_TRUE:
			//	return "true";
			//case OSCOut::NT_FALSE:
			//	return "false";
			//case OSCOut::NT_NIL:
			//	return "nil";
			//case OSCOut::NT_INF:
			//	return "inf";
			}

			return "UNKNOWN";
		}

		OSCOut::NativeType nativeTypeFromString( const char *s )
		{
			if( !s )
				return OSCOut::NT_COUNT;

			for( int i = 0; i < OSCOut::NT_COUNT; i++ )
				if( !_stricmp( s, nativeTypeToString( ( OSCOut::NativeType ) i ) ) )
					return ( OSCOut::NativeType ) i;

			return OSCOut::NT_COUNT;
		}

		OSCOut::NativeType nativeTypeFromString( const std::string &s )
		{
			return nativeTypeFromString( s.c_str() );
		}

		char loFromNativeType( OSCOut::NativeType nt )
		{
			switch( nt )
			{
			case OSCOut::NT_INT32: 
				return LO_INT32;
			case OSCOut::NT_FLOAT: 
				return LO_FLOAT;
			//case OSCOut::NT_STRING: 
			//	return LO_STRING;
			//case OSCOut::NT_BLOB: 
			//	return LO_BLOB;

			case OSCOut::NT_INT64: 
				return LO_INT64;
			//case OSCOut::NT_TIMETAG: 
			//	return LO_TIMETAG;
			case OSCOut::NT_DOUBLE: 
				return LO_DOUBLE;
			//case OSCOut::NT_SYMBOL: 
			//	return LO_SYMBOL;
			case OSCOut::NT_CHAR: 
				return LO_CHAR;
			//case OSCOut::NT_MIDI: 
			//	return LO_MIDI;
			//case OSCOut::NT_TRUE: 
			//	return LO_TRUE;
			//case OSCOut::NT_FALSE: 
			//	return LO_FALSE;
			//case OSCOut::NT_NIL: 
			//	return LO_NIL;
			//case OSCOut::NT_INF: 
			//	return LO_INFINITUM;
			}

			throw std::runtime_error( "unknown lo type" );
		}



		OSCOut::OSCOut( const std::string &ip, unsigned short port, const std::string &oscPath, unsigned char sensorID, unsigned char deviceID ) :
			Op(),
			_sending( false ),
			_proto( P_UDP ),
			_ip( ip ),
			_port( port ),
			_path( oscPath ),
			_outType( OT_SAMPLEFRAME ),
			_nativeType( NT_FLOAT ),
			_deviceID( deviceID ),
			_sensorID( sensorID ),
			_inputBufferIP( 32 ),
			_inputBufferPort( 16 ),
			_inputBufferPath( 256 ),
			_address( nullptr )
		{
			updateBuffers();
			open();
		}

		OSCOut::~OSCOut()
		{
			close();
		}

		bool OSCOut::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				if( _sending )
				{
					if( _address )
					{
						if( sf->size() > PACKAGE_MAX_SAMPLES && _proto == P_UDP )
							std::cout << "<error> sampleframe with " << sf->size() << " samples is most certainly too tall to be sent via OSC -- package will not be sent" << std::endl;
						else
						{
							if( sf->size() > 100 && _proto == P_UDP )
							{
								//NOTE: according to RFC 791 [1], only 576 bytes are guaranteed (subtract osc overhead and custom sampleframe header to get to something 
								// like ~120 samples). if you go beyond that, make sure
								// - the receiver can handle the data (check input package sizes and watch out for input buffer overflow)
								// - output buffer does not overflow
								// - the sender can handle packages of this size without having to fragment
								// [1] https://tools.ietf.org/html/rfc791 
								std::cout << "<warning> sampleframe size is borderline for sending OSC packages with " << sf->size() << " samples." << std::endl;
							}

							bool ret = true;

							lo_message message = lo_message_new();

							try
							{
								std::string path( _path );

								if( _outType == OT_SAMPLEFRAME )
								{
									lo_blob blob = nullptr;

									//NOTE: using int32 here, as int8 are non-standard [1] in OSC and cause 
									// issues with various OSC implementations on receiver's end. also, char 
									// is sent as 4-byte int anyways [1], so it doesn't make a difference to 
									// boot.
									// [1] http://opensoundcontrol.org/spec-1_0
									lo_message_add_int32( message, (int32_t) _deviceID );
									lo_message_add_int32( message, (int32_t) _sensorID );
									//-----

									lo_message_add_int32( message, (int32_t) sf->width() );
									lo_message_add_int32( message, (int32_t) sf->height() );
									lo_message_add_int32( message, (int32_t) sf->depth() );
									lo_message_add_int32( message, (int32_t) sf->timeStamp() );

									if( sf->size() )
										blob = lo_blob_new( sf->size() * sizeof( float ), &sf->values()[0] );
									else
									{
										//cannot send empty blob (but prefer to send blob since receivers mostly validate by number of OSC arguments)
										float dummy = 0;
										blob = lo_blob_new( sizeof( float ), &dummy );
									}
									lo_message_add_blob( message, blob );

									if( blob )
										lo_blob_free( blob );
								}
								else if( _outType == OT_NATIVE )
								{
									const float *ptr = sf->values();
									for( int i = 0; i < sf->size(); i++, ptr++ )
									{
										//TODO: doublecheck if sf is too big for osc package, and/or handle error coming from lo_message_add*
										switch( _nativeType )
										{
										case NT_INT32:
											lo_message_add_int32( message, (int32_t) *ptr );
											break;
										case NT_FLOAT:
											lo_message_add_float( message, (float) *ptr );
											break;
										case NT_INT64:
											lo_message_add_int64( message, (int64_t) *ptr );
											break;
										case NT_DOUBLE:
											lo_message_add_double( message, (double) *ptr );
											break;
										case NT_CHAR:
											lo_message_add_char( message, (char) *ptr );
											break;
										default:
											throw std::runtime_error( "type not implemented" );
										}
									}
								}
								else
								{
									throw std::runtime_error( "OT_NATIVE_CUSTOM not implemented" );
								}

								if( lo_send_message( _address, path.c_str(), message ) < 0 )
								{
									std::cerr << "<error> failed sending OSC message (UDP)" << std::endl;
									close();
								}
							}
							catch( std::exception &e )
							{
								std::cerr << "<error> unable to send " << _path << " (" << _ip << ":" << _port << "): " << e.what() << std::endl;
								ret = false;
							}

							if( message )
								lo_message_free( message );
						}
					}
					else
					{
						std::cerr << "<error> cannot send, socket not set up" << std::endl;
						setEnabled( false );
					}
				}

				drawFrame( sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool OSCOut::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _sending ? "stop" : "start" ) )
				_sending = !_sending;

			{
				ScopedImGuiDisable disable( _sending );

				int p = (int) _proto;
				for( int i = 0; i < FF_COUNT; i++ )
					ImGui::RadioButton( protocolToString( (Protocol) i ), &p, i );
				if( _proto != (Protocol) p )
				{
					_proto = (Protocol) p;

					open();
				}

				if( ImGui::InputText( "ip", &_inputBufferIP[0], _inputBufferIP.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					bool valid = true;

					std::string str = trim( &_inputBufferIP[0] );
					std::vector<std::string> subs = split( str, '.' );
					if( subs.size() != 4 )
						valid = false;
					else for( auto it = subs.begin(); it != subs.end(); ++it )
						for( auto it2 = it->begin(); it2 != it->end(); ++it2 )
							if( *it2 < '0' || *it2 > '9' )
								valid = false;

					if( valid )
					{
						_ip = str;

						open();
					}
					else
						strcpy( &_inputBufferIP[0], _ip.c_str() );

					updateBuffers();
				}

				if( ImGui::InputText( "port", &_inputBufferPort[0], _inputBufferPort.size(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					_port = atoi( &_inputBufferPort[0] );

					open();

					updateBuffers();
				}

				if( ImGui::InputText( "path", &_inputBufferPath[0], _inputBufferPath.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					std::string str = trim( &_inputBufferPath[0] );
					if( str.size() )
					{
						if( str[0] != '/' )
							_path = "/" + str;
						else
							_path = str;
					}
					else
						_path = "/";

					updateBuffers();
				}

				bool native = ( _outType == OT_NATIVE || _outType == OT_NATIVE_CUSTOM );
				bool custom = ( _outType == OT_NATIVE_CUSTOM );
				ImGui::Checkbox( "OSC native types", &native );
				ImGui::SameLine();
				ImGui::BeginDisabled();
				ImGui::Checkbox( "OSC custom", &custom );
				ImGui::EndDisabled();

				if( native )
				{
					if( custom )
					{
						//TODO: implement
						//_outType = OT_NATIVE_CUSTOM;
					}
					else
					{
						_outType = OT_NATIVE;

						int e = (int)_nativeType;
						for( int i = 0; i < (int)OSCOut::NT_COUNT; i++ )
							if( ImGui::RadioButton( nativeTypeToString( (OSCOut::NativeType)i ), i == e ) )
								_nativeType = (OSCOut::NativeType)i;
					}
				}
				else
				{
					_outType = OT_SAMPLEFRAME;

					int i = _deviceID;
					if( ImGui::InputInt( "device ID", &i ) )
						_deviceID = i;
					i = _sensorID;
					if( ImGui::InputInt( "sensor ID", &i ) )
						_sensorID = i;
				}
			}

			return true;
		}
#endif

		void OSCOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

		bool OSCOut::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "sending", _sending );
			std::string str;
			if( load<std::string>( j, "proto", str ) )
				_proto = protocolFromString( str );
			load<std::string>( j, "ip", _ip );
			load<uint16_t>( j, "port", _port );
			load<std::string>( j, "oscPath", _path );
			if( load<std::string>( j, "outType", str ) )
				_outType = outTypeFromString( str );
			if( load<std::string>( j, "nativeType", str ) )
				_nativeType = nativeTypeFromString( str );
			load<unsigned char>( j, "deviceID", _deviceID );
			load<unsigned char>( j, "sensorID", _sensorID );

			open();
			updateBuffers();

			return ret;
		}

		bool OSCOut::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "sending", _sending );
			save( j, "proto", protocolToString( _proto ) );
			save( j, "ip", _ip );
			save( j, "port", _port );
			save( j, "oscPath", _path );
			save( j, "outType", outTypeToString( _outType ) );
			save( j, "nativeType", nativeTypeToString( _nativeType ) );
			save( j, "deviceID", _deviceID );
			save( j, "sensorID", _sensorID );

			return ret;
		}

		void OSCOut::updateBuffers()
		{
			strncpy( &_inputBufferIP[0], _ip.c_str(), _ip.size() + 1 );
			snprintf( &_inputBufferPort[0], _inputBufferPort.size() - 1, "%d", _port );
			strncpy( &_inputBufferPath[0], _path.c_str(), _path.size() + 1 );
		}

		bool OSCOut::open()
		{
			close();

			int proto = LO_DEFAULT;

			switch( _proto )
			{
			case P_UDP:
				proto = LO_UDP;
				break;
			case P_TCP:
				proto = LO_TCP;
				break;
			default:
				std::cerr << "<error> unknown protocol" << std::endl;
				return false;
			}

			_address = lo_address_new_with_proto( proto, _ip.c_str(), toString( _port ).c_str() );
			if( !_address )
			{
				std::cerr << "<error> creating address " << _ip << ":" << _port << " failed (" << protocolToString( _proto ) << ")" << std::endl;
				return false;
			}
			
			std::cout << "created osc socket " << _ip << ":" << _port << "(" << protocolToString( _proto ) << ")" << std::endl;
			if( _proto == P_TCP )
			{
				std::cout << "sending TCP test package, trying to connect..." << std::endl;
				if( lo_send( _address, "/", "" ) < 0 )	//send dummy message to see if there is a server at all, otherwise sending in process() would block for quite a while
				{
					std::cerr << "<error> sending TCP test package failed, closing again." << std::endl;
					close();
					return false;
				}
			}

			return true;
		}

		void OSCOut::close()
		{
			if( _address )
			{
				lo_address_free( _address );
				_address = nullptr;
			}
		}
	}
}