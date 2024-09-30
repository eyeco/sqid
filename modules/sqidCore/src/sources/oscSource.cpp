/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "oscSource.h"

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
				deviceID( 0xff ), sensorID( 0xff ),
				width( 1 ), height( 1 ), depth( 1 ),
				timestamp( timestamp ),
				values( 1 )
			{
				this->values[0] = f;
			}

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
			unsigned int _maxQueueSize;
			std::list<OSCMsg*> _msgQueue;
			std::mutex _msgMutex;

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
				return reinterpret_cast<OSCSourceImpl*>( userData )->messageHandler( path, types, argv, argc, msg );
			}

			int messageHandler( const char *path, const char *types, lo_arg ** argv, int argc, lo_message msg )
			{
				try
				{
					lo_address srcAddr = lo_message_get_source( msg );
					const char *srcHost = lo_address_get_hostname( srcAddr );
					//const char *srcPort = lo_address_get_port( srcAddr );

					if( argc == 1 )
					{
						//TODO: apart from sampleframes, support single int, floats, etc... maybe let user specify type pattern (maybe even dest-type/clipping/scaling) and create Nx1 SampleFrame from it

						float f = 0.0f;

						//TODO: not sure if we can use OSC timing data -- may get us into trouble in progessing in scenegraph. maybe let user decide?
						uint32_t ts = getAppTime() * 1000;

						switch( types[0] )
						{
						case 'c':	//non-OSC-standard datatype, actually.
							f = argv[0]->c / 255.0f;
							break;
						case 'i':
							//TODO: provide SampleFrame<int> type, but until then simply treat as float...
							f = argv[0]->i32;
							break;
						case 'f':
							f = argv[0]->f32;
							break;
						default:
							std::cerr << "<error> invalid datatype '" << types[0] << "' (or type not yet implemented)" << std::endl;
							return -1;
						}

						{
							std::lock_guard<std::mutex> lock( _msgMutex );
							_msgQueue.push_back( new OSCMsg( std::string( srcHost ), std::string( path ), f, ts ) );
						}
					}
					else if( argc == 7 )
					{
						//types = "iiiiiib"
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
							std::lock_guard<std::mutex> lock( _msgMutex );
							_msgQueue.push_back( new OSCMsg( std::string( srcHost ), std::string( path ), deviceID, sensorID, width, height, depth, values, ts ) );
						}
					}
					else
					{
						std::cerr << "<error> unexpected number of OSC arguments: " << argc << std::endl;
						return -1;
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
			OSCSourceImpl() :
				_maxQueueSize( SOURCE_MAX_QUEUE_SIZE ),
				_isStarted( false ),
				_keepRunning( false ),
				_server( nullptr ),
				_oscListenerThread( nullptr )
			{}

			~OSCSourceImpl()
			{
				close();
			}

			bool run( const std::string &ip, unsigned short port, Protocol proto, unsigned int queueSize )
			{
				if( _isStarted || _oscListenerThread || _server )
					return false;

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
					lo_server_add_method( _server, NULL, NULL, OSCSourceImpl::messageHandler, this );

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
					lo_server_add_method( _server, NULL, NULL, OSCSourceImpl::messageHandler, this );

					break;
				default:
					std::cerr << "<error> unknown protocol" << std::endl;
					return false;
				}

				_keepRunning = true;

				std::stringstream sstr;
				sstr << protocolToString( proto ) << "://" << ip << ":" << port;
				_desc = sstr.str();

				_oscListenerThread = new std::thread( &OSCSourceImpl::oscListen, this );

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

						frames.push_back( SampleFrameContainer( msg->deviceID, msg->sensorID, msg->addressPattern, new SampleFrame( msg->width, msg->height, &msg->values[0], msg->timestamp, msg->depth ) ) );

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
					ImGui::Text( "queued: %d", _msgQueue.size() );

					//if( ImGui::TreeNode( "senders", "%d senders", _activeSenders.size() ) )
					//{
					//	for( auto &it : _activeSenders )
					//	{
					//		char deviceID = ( it >> 8 ) & 0xff;
					//		char sensorID = ( it ) & 0xff;

					//		ImGui::Text( "  dID: %d, sID: %d", deviceID, sensorID );
					//	}

					//	ImGui::TreePop();
					//}

					//_activeSenders.clear();
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



	//bool OSCSource::init()
	//{
	//	return Internal::OSCSourceImpl::init();
	//}

	//bool OSCSource::isInitialized()
	//{
	//	return Internal::OSCSourceImpl::isInitialized();
	//}




	OSCSource::OSCSource() :
		DataSource(),
		_proto( OSC_DEFAULT_PROTO ),
		_ip( "127.0.0.1" ),	//TODO: neable to choose network adapter if multiple are present
		_port( OSC_SOURCE_DEFAULT_PORT ),
		_queueSize( SOURCE_MAX_QUEUE_SIZE ),
		_connected( false ),
		_inputBufferIP( 32 ),
		_impl( new Internal::OSCSourceImpl() )
	{
		updateBuffers();
	}

	OSCSource::~OSCSource()
	{
		close();

		safeDelete( _impl );
	}

	bool OSCSource::run()
	{
		if( _connected )
			close();

		if( _impl->run( _ip, _port, _proto, _queueSize ) )
			_connected = true;
		else
			std::cerr << "<error> opening OSC at " << _ip << ":" << _port << " failed" << std::endl;
	
		return false;
	}

	void OSCSource::close()
	{
		_impl->close();
		_connected = false;
	}

	void OSCSource::fetchFrames( std::vector<SampleFrameContainer> &frames )
	{
		_impl->fetchFrames( frames );
	}

	std::string OSCSource::getDesc() const
	{
		return _impl->getDesc();
	}

#ifdef __SUPPORT_GUI
	bool OSCSource::drawUI()
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

			int s = _queueSize;
			if( ImGui::InputInt( "max queue size", &s ) )
				_queueSize = s;
		}

		_impl->drawUI();

		return true;
	}
#endif

	void OSCSource::updateBuffers()
	{
		strncpy( &_inputBufferIP[0], _ip.c_str(), _ip.size() + 1 );
	}

	bool OSCSource::loadFromJSON( const nlohmann::json &j )
	{
		if( !DataSource::loadFromJSON( j ) )
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

	void OSCSource::saveToJSON( nlohmann::json &j ) const
	{
		DataSource::saveToJSON( j );

		save( j, "proto", protocolToString( _proto ) );
		save( j, "ip", _ip );
		save( j, "port", _port );
		save( j, "queueSize", _queueSize );
		save( j, "connected", _connected );
	}
}