/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "oscInterface.h"

#include <fileIO/json.h>

#include <commonImGui.h>

#include <app.h>

#include <map>
#include <list>
#include <mutex>
#include <thread>
#include <iostream>

#include <imgui/imgui.h>

#include <lo/lo.h>

#define OSC_DEFAULT_PROTO		P_UDP
#define OSC_SOURCE_DEFAULT_PORT 6667

namespace sqid
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
			OSCMsg( const std::string &senderIP, const std::string &addressPattern, float f, uint32_t timestamp ) :
				senderIP( senderIP ), addressPattern( addressPattern ),
				width( 1 ), height( 1 ), depth( 1 ),
				timestamp( timestamp ),
				values( 1 )
			{
				this->values[0] = f;
			}

			OSCMsg( const std::string &senderIP, const std::string &addressPattern, unsigned short width, unsigned short height, unsigned short depth, const float *values, uint32_t timestamp ) :
				senderIP( senderIP ), addressPattern( addressPattern ),
				width( width ), height( height ), depth( depth ),
				timestamp( timestamp ),
				values( width * height * depth )
			{
				if( this->values.size() )
					memcpy( &this->values[0], values, this->values.size() * sizeof( float ) );
			}

			std::string senderIP;
			std::string addressPattern;

			unsigned short width;
			unsigned short height;
			unsigned short depth;

			uint32_t timestamp;

			std::vector<float> values;
		};

		class OSCInterfaceImpl
		{
		private:
			unsigned int _maxQueueSize;
			std::list<OSCMsg*> _msgQueue;
			std::mutex _msgMutex;

			std::set<std::string> _activeSenders;

			std::string _desc;

			bool _isStarted;
			bool _keepRunning;

			lo_server _server;

			std::thread *_oscListenerThread;

			void oscListen()
			{
				while( _keepRunning )
				{
					if( _server )
						lo_server_recv_noblock( _server, 1 );
					else
						break;
				}
			}

			static int messageHandler( const char *path, const char *types, lo_arg ** argv, int argc, lo_message msg, void *userData )
			{
				return reinterpret_cast<OSCInterfaceImpl*>( userData )->messageHandler( path, types, argv, argc, msg );
			}

			int messageHandler( const char *path, const char *types, lo_arg ** argv, int argc, lo_message msg )
			{
				try
				{
					lo_address srcAddr = lo_message_get_source( msg );
					const char *srcHost = lo_address_get_hostname( srcAddr );
					//const char *srcPort = lo_address_get_port( srcAddr );

					bool isSF = false;
					if( argc == 5 )
					{
						isSF = true;

						if( types[4] != LO_BLOB )
							isSF = false;
						for( int i = 0; i < 4; i++ )
							if( types[i] != LO_INT32 )
							{
								//still accepting 'c', altough not entirely correct
								if( types[i] != LO_CHAR )
									isSF = false;
								else
									std::cerr << "<warning> expected types '" 
										<< LO_INT32 << LO_INT32 << LO_INT32 << LO_INT32 << LO_BLOB << "', got '" 
										<< types[0] << types[1] << types[2] << types[3] << types[4] << "', trying to treat as sampleframe message..." << std::endl;
							}

						if( isSF )
						{
							unsigned short width = (unsigned short) ( argv[0]->i32 );
							unsigned short height = (unsigned short) ( argv[1]->i32 );
							unsigned short depth = (unsigned short) ( argv[2]->i32 );
							uint32_t ts = (uint32_t) ( argv[3]->i32 );

							const float* values = reinterpret_cast<float*>( &argv[4]->blob.data );
							unsigned int size = width * height * depth;
							if( argv[4]->blob.size != size * sizeof( float ) )
							{
								std::cerr << "<warning> OSC blob has unexpected size" << std::endl;
								return -1;
							}

							{
								std::lock_guard<std::mutex> lock( _msgMutex );
								_msgQueue.push_back( new OSCMsg( std::string( srcHost ), std::string( path ), width, height, depth, values, ts ) );

								_activeSenders.insert( std::string( path ) );
							}
						}
					}

					if( !isSF )
					{
						//TODO: consider using OSC timing data -- may get us into trouble in progessing in scenegraph. maybe let user decide?
						uint32_t ts = getAppTime() * 1000;

						std::vector<float> v;
						for( int i = 0; i < argc; i++ )
						{
							switch( types[i] )
							{
								//standard types
							case LO_INT32:
								v.push_back( static_cast<float>( argv[i]->i32 ) );
								break;
							case LO_FLOAT:
								v.push_back( argv[i]->f32 );
								break;
							case LO_STRING:
								std::cerr << "<warning> OSC message contains string -- trying to convert to float" << std::endl;
								v.push_back( static_cast<float>( std::stof( &( argv[i]->s ) ) ) );
								break;
							case LO_BLOB:
								std::cerr << "<error> invalid datatype 'b' (blob), cannot convert to float" << std::endl;
								break;

								//extended types
							case LO_INT64:
								v.push_back( static_cast<float>( argv[i]->i64 ) );
								break;
							case LO_TIMETAG:
								lo_timetag tt = argv[i]->t;
								v.push_back( static_cast<float>( tt.sec ) + static_cast<float>( tt.frac ) / 0xffffffff ); //convert to seconds
								break;
							case LO_DOUBLE:
								std::cerr << "<warning> truncating double to float" << std::endl;
								v.push_back( static_cast<float>( argv[i]->f64 ) );
								break;
							case LO_SYMBOL:
								std::cerr << "<error> invalid datatype 'S' (symbol), cannot convert to float" << std::endl;
								break;
							case LO_CHAR:
								v.push_back( static_cast<float>( argv[i]->c ) );
								break;
							case LO_MIDI:
								v.push_back( static_cast<float>( argv[i]->m[0] ) );
								v.push_back( static_cast<float>( argv[i]->m[1] ) );
								v.push_back( static_cast<float>( argv[i]->m[2] ) );
								v.push_back( static_cast<float>( argv[i]->m[3] ) );
								break;
							case LO_TRUE:
								v.push_back( 1.0f );
								break;
							case LO_FALSE:
								v.push_back( 0.0f );
								break;
							case LO_NIL:
								v.push_back( std::numeric_limits<float>::quiet_NaN() );
								break;
								/** Sybol representing the value Infinitum. */
							case LO_INFINITUM:
								v.push_back( std::numeric_limits<float>::infinity() );
								break;
							default:
								std::cerr << "<error> unknown data type '" << types[i] << "' (or type not yet implemented)" << std::endl;
							}
						}

						if( v.size() )
						{
							std::lock_guard<std::mutex> lock( _msgMutex );
							_msgQueue.push_back( new OSCMsg( std::string( srcHost ), std::string( path ), v.size(), 1, 1, &v[0], ts ) );

							_activeSenders.insert( std::string( path ) );
						}
						else
						{
							std::cerr << "<error> invalid format" << std::endl;
							return -1;
						}
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
			OSCInterfaceImpl() :
				_maxQueueSize( SOURCE_MAX_QUEUE_SIZE ),
				_isStarted( false ),
				_keepRunning( false ),
				_server( nullptr ),
				_oscListenerThread( nullptr )
			{}

			~OSCInterfaceImpl()
			{
				close();
			}

			bool run( const std::string &ip, unsigned short port, Protocol proto, unsigned int queueSize )
			{
				if( _isStarted || _oscListenerThread || _server )
					return false;
				_isStarted = true;

				_maxQueueSize = queueSize;

				switch( proto )
				{
				case P_UDP:
					std::cerr << "trying to open OSC on port " << port << " (UDP)" << std::endl;
					_server = lo_server_new( toString( port ).c_str(), loErrorHandler );

					if( !_server )
					{
						std::cerr << "<error> creating server failed" << std::endl;
						return false;
					}

					//NOTE: will do filtering on my own since i want it to be caseINsensitive (also there is a setter which will enable to change the filter at a later point)
					lo_server_add_method( _server, NULL, NULL, OSCInterfaceImpl::messageHandler, this );

					break;
				case P_TCP:
					std::cerr << "trying to open OSC at " << ip << ":" << port << " (TCP)" << std::endl;
					_server = lo_server_new_with_proto( toString( port ).c_str(), LO_TCP, loErrorHandler );

					if( !_server )
					{
						std::cerr << "<error> creating server failed" << std::endl;
						return false;
					}

					//NOTE: will do filtering on my own since i want it to be caseINsensitive (also there is a setter which will enable to change the filter at a later point)
					lo_server_add_method( _server, NULL, NULL, OSCInterfaceImpl::messageHandler, this );

					break;
				default:
					std::cerr << "<error> unknown protocol" << std::endl;
					return false;
				}

				_keepRunning = true;

				std::stringstream sstr;
				sstr << protocolToString( proto ) << "://" << ip << ":" << port;
				_desc = sstr.str();

				_oscListenerThread = new std::thread( &OSCInterfaceImpl::oscListen, this );

				return true;
			}

			void close()
			{
				_keepRunning = false;
				if( _oscListenerThread )
				{
					_oscListenerThread->join();

					safeDelete( _oscListenerThread );
				}
				if( _server )
				{
					lo_server_free( _server );
					_server = nullptr;
				}

				{
					std::lock_guard<std::mutex> lock( _msgMutex );
					for( auto it : _msgQueue )
						safeDelete( it );
					_msgQueue.clear();

					_activeSenders.clear();
				}

			}

			std::string getDesc() const
			{
				return _desc;
			}

			void fetchFrames( std::vector<SampleFrameContainer> &frames )
			{
				if( frames.size() )
					std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory here!" << std::endl;

				if( _msgQueue.size() )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					while( _msgQueue.size() )
					{
						OSCMsg *msg = _msgQueue.front();

						frames.push_back( SampleFrameContainer( msg->addressPattern, new SampleFrame( msg->width, msg->height, &msg->values[0], msg->timestamp, msg->depth ) ) );

						safeDelete( msg );
						_msgQueue.pop_front();
					}
				}
			}

			unsigned int getMaxQueueSize() const { return _maxQueueSize; }

#ifdef __SUPPORT_GUI
			bool drawUI()
			{
				ImGui::Text( _desc.c_str() );

				if( _isStarted )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					ImGui::Text( "queued: %d", _msgQueue.size() );

					if( ImGui::TreeNode( "senders", "%d senders", _activeSenders.size() ) )
					{
						for( auto &it : _activeSenders )
							ImGui::Text( "  msg: %s", it.c_str() );

						ImGui::TreePop();
					}

					_activeSenders.clear();
				}

				return true;
			}
#endif

			//static bool init()
			//{
			//}

			//static bool isInitialized()
			//{
			//}
		};
	}



	//bool OSCInterface::init()
	//{
	//	return Internal::OSCSourceImpl::init();
	//}

	//bool OSCInterface::isInitialized()
	//{
	//	return Internal::OSCSourceImpl::isInitialized();
	//}




	OSCInterface::OSCInterface() :
		DataInterface(),
		_proto( OSC_DEFAULT_PROTO ),
		_ip( "127.0.0.1" ),	//TODO: neable to choose network adapter if multiple are present
		_port( OSC_SOURCE_DEFAULT_PORT ),
		_queueSize( SOURCE_MAX_QUEUE_SIZE ),
		_connected( false ),
		_inputBufferIP( 32 ),
		_impl( new Internal::OSCInterfaceImpl() )
	{
		updateBuffers();
	}

	OSCInterface::~OSCInterface()
	{
		close();

		safeDelete( _impl );
	}

	bool OSCInterface::run()
	{
		if( _connected )
			close();

		if( _impl->run( _ip, _port, _proto, _queueSize ) )
			_connected = true;
		else
			std::cerr << "<error> opening OSC at " << _ip << ":" << _port << " failed" << std::endl;
	
		return false;
	}

	void OSCInterface::close()
	{
		_impl->close();
		_connected = false;
	}

	bool OSCInterface::doesWant( const SampleFrameContainer *sfc ) const
	{
		//return ( _exactOSC ? sfc->message.compare( _msgFilterOSC ) == 0 : sfc->message.compare( 0, _msgFilterOSC.size(), _msgFilterOSC ) == 0 );
		//not implemented for now, better to use oscOut operator
		return false;
	}

	void OSCInterface::fetchFrames( std::vector<SampleFrameContainer> &frames )
	{
		_impl->fetchFrames( frames );
	}

	bool OSCInterface::queueFrame( const SampleFrameContainer& sfc )
	{
		std::cerr << "<error> sending frames to OSCInterface not implemented (yet) -- use oscOut operator" << std::endl;
		return false;
	}

	std::string OSCInterface::getDesc() const
	{
		return _impl->getDesc();
	}

#ifdef __SUPPORT_GUI
	bool OSCInterface::drawUI()
	{
		if( ImGui::Button( _connected ? "disconnect" : "connect" ) )
		{
			if( _connected )
				close();
			else
				run();
		}

		{
			ScopedImGuiDisable disable( _connected );

			int p = (int) _proto;
			for( int i = 0; i < FF_COUNT; i++ )
				ImGui::RadioButton( protocolToString( (Protocol) i ), &p, i );
			if( _proto != (Protocol) p )
				_proto = (Protocol) p;

			{
				ScopedImGuiDisable disableIP( _proto == P_UDP );

				{
					ScopedImGuiStyleColor redText( ImGuiCol_Text, ImVec4( 1, 0, 0, 1 ), strcmp( _ip.c_str(), &_inputBufferIP[0] ) );
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
			}

			int i = _port;
			if( ImGui::InputInt( "port", &i ) )
				_port = clamp<int>( i, 0x0000, 0xffff );

			int s = _queueSize;
			if( ImGui::InputInt( "max queue size", &s ) )
				_queueSize = s;
		}

		_impl->drawUI();

		return true;
	}
#endif

	void OSCInterface::updateBuffers()
	{
		strncpy( &_inputBufferIP[0], _ip.c_str(), _ip.size() + 1 );
	}

	bool OSCInterface::loadFromJSON( const nlohmann::json &j )
	{
		if( !DataInterface::loadFromJSON( j ) )
			return false;

		close();

		std::string str;
		if( load<std::string>( j, "proto", str ) )
			_proto = protocolFromString( str );
		load<std::string>( j, "ip", _ip );
		load<unsigned short>( j, "port", _port );
		load<unsigned int>( j, "queueSize", _queueSize );
		load<bool>( j, "connected", _connected );
		
		if( _connected )
			run();

		return true;
	}

	void OSCInterface::saveToJSON( nlohmann::json &j ) const
	{
		DataInterface::saveToJSON( j );

		save( j, "proto", protocolToString( _proto ) );
		save( j, "ip", _ip );
		save( j, "port", _port );
		save( j, "queueSize", _queueSize );
		save( j, "connected", _connected );
	}
}