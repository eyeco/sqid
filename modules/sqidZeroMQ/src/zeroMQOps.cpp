/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "zeroMQOps.h"
#include "sqidZeroMQ.h"
#include "zeroMQContext.h"

#include <processing/opFactory.h>

#include <app.h>

#include <commonImGui.h>

#include <zmq.h>

#define ZMQ_INPUT_BUFFER_SIZE	( 0x01 << 12 )

namespace sqid
{
	namespace Plugins
	{
		DEFINE_OP_DESC( ZMQIn, "zmqIn", "/networking",
			"FAABF749-BD55-4F09-81E0-F5E96C8E1441" );
		DEFINE_OP_DESC( ZMQOut, "zmqOut", "/networking",
			"220AA88D-37C7-43E5-A0E2-C41072ED586B" );

#pragma pack(push)
#pragma pack(1)
		struct ZMQHdr
		{
			unsigned short w;
			unsigned short h;
			unsigned short d;

			unsigned int ts;
		};
#pragma pack(pop)

		struct ZMQMsg
		{
			ZMQHdr hdr;
			std::vector<unsigned char> data;
		};

		class ZMQSourceImpl
		{
		private:
			enum State
			{
				S_ERROR,

				S_TOPIC,
				S_HEADER,
				S_FRAME,

				S_COUNT
			};

			State _state;
			std::vector<unsigned char> _buffer;
			size_t _bufferSize;

			ZMQHdr _hdrAwaited;

			std::mutex _mutex;
			std::thread* _thread;
			bool _keepRunning;

			std::list<SampleFrame*> _queue;

			void* _socket;

			void listen()
			{
				while( _keepRunning )
				{
					if( _socket )
					{
						if( _bufferSize == _buffer.size() )
						{
							std::cerr << "<error> input buffer overrun" << std::endl;
							_bufferSize = 0;
						}

						int ret = zmq_recv( _socket, &_buffer[_bufferSize], _buffer.size() - _bufferSize, ZMQ_NOBLOCK );
						if( ret > 0 )
						{
							_bufferSize += ret;

							if( _state == S_TOPIC )
							{
								const unsigned char* ptr = &_buffer[0];
								const unsigned char* end = ptr + _bufferSize;

								while( ptr < end )
								{
									if( !( *( ptr++ ) ) )	//find end of topic, beginning of header info
									{
										_bufferSize = end - ptr;
										if( _bufferSize )
										{
											//std::cout << _bufferSize << " bytes left in buffer, need to re-align (TOPIC)" << std::endl;
											memcpy( &_buffer[0], ptr, _bufferSize );
										}
										_state = S_HEADER;
										break;
									}
								}
							}

							if( _state == S_HEADER )
							{
								if( _bufferSize >= sizeof( ZMQHdr ) )
								{
									_hdrAwaited = *( reinterpret_cast<ZMQHdr*>( &_buffer[0] ) );

									_bufferSize -= sizeof( ZMQHdr );
									if( _bufferSize )
									{
										//std::cout << _bufferSize << " bytes left in buffer, need to re-align (HEADER)" << std::endl;
										memcpy( &_buffer[0], &_buffer[sizeof( ZMQHdr )], _bufferSize );
									}
									_state = S_FRAME;
								}
							}

							if( _state == S_FRAME )
							{
								size_t awaitedSize = _hdrAwaited.w * _hdrAwaited.h * _hdrAwaited.d * sizeof( float );

								if( !awaitedSize )
									std::cerr << "<warning> size of awaited frame is 0" << std::endl;

								if( _bufferSize >= awaitedSize )
								{
									SampleFrame* sf = new SampleFrame( _hdrAwaited.w, _hdrAwaited.h, reinterpret_cast<const float*>( &_buffer[0] ), _hdrAwaited.ts, _hdrAwaited.d );

									{
										std::lock_guard<std::mutex> lock( _mutex );

										_queue.push_back( sf );
									}

									_bufferSize -= awaitedSize;
									if( _bufferSize )
									{
										//std::cout << _bufferSize << " bytes left in buffer, need to re-align (FRAME)" << std::endl;
										memcpy( &_buffer[0], &_buffer[awaitedSize], _bufferSize );
									}
									_state = S_TOPIC;
								}
							}
						}
						else if( ret < 0 )
						{
							if( zmq_errno() == EAGAIN )
							{
								Sleep( 1 );
								continue;
							}

							std::cerr << "<error> failed receiving from ZMQ: " << zmq_strerror( zmq_errno() ) << std::endl;
							break;
						}
					}
					else
						break;
				}
			}

		protected:

		public:
			explicit ZMQSourceImpl() :
				_state( S_TOPIC ),
				_buffer( ZMQ_INPUT_BUFFER_SIZE ),
				_bufferSize( 0 ),
				_thread( nullptr ),
				_keepRunning( false ),
				_socket( nullptr )
			{}

			~ZMQSourceImpl()
			{
				close();
			}

			bool run( const std::string& ip, unsigned short port, const std::string& topic )
			{
				if( _socket || _thread )
					return false;

				_state = S_ERROR;

				auto ctxt = ZMQSingleton::get();
				if( !ctxt || !ctxt->ctx() )
				{
					std::cerr << "<error> zmqContext not initialized!" << std::endl;
					return false;
				}

				std::cout << "creating ZMQ socket" << std::endl;

				_socket = zmq_socket( ctxt->ctx(), ZMQ_SUB );
				if( !_socket )
				{
					std::cerr << "<error> failed to create socket: " << zmq_strerror( zmq_errno() ) << std::endl;
					return false;
				}

				char tempStr[128];
				sprintf( tempStr, "tcp://%s:%d", ip.c_str(), port );

				std::cout << "connecting ZMQ socket to " << tempStr << std::endl;

				if( zmq_connect( _socket, tempStr ) < 0 )
				{
					std::cerr << "<error> failed to connect to " << tempStr << ": " << zmq_strerror( zmq_errno() ) << std::endl;
					return false;
				}

				std::cout << "subscribing to topic " << topic << std::endl;

				std::string t( "t:" );
				t.append( topic );

				zmq_setsockopt( _socket, ZMQ_SUBSCRIBE, t.c_str(), t.length() );

				_keepRunning = true;
				_state = S_TOPIC;
				_bufferSize = 0;

				_thread = new std::thread( &ZMQSourceImpl::listen, this );

				return true;
			}

			void close()
			{
				_keepRunning = false;
				if( _thread )
				{
					_thread->join();
					safeDelete( _thread );
				}
				if( _socket )
				{
					zmq_close( _socket );
					_socket = nullptr;
				}

				for( auto it : _queue )
					safeDelete( it );
				_queue.clear();
			}

			bool fetchFrames( std::vector<SampleFrame*>& frames )
			{
				if( frames.size() )
					std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory here!" << std::endl;

				if( _queue.size() )
				{
					std::lock_guard<std::mutex> lock( _mutex );

					while( _queue.size() )
					{
						frames.push_back( _queue.front() );
						_queue.pop_front();
					}

					return true;
				}

				return false;
			}
		};




		ZMQIn::ZMQIn( const std::string& ip, unsigned short port, const std::string& topic ) :
			Op(),
			_impl( nullptr ),
			_ip( ip ),
			_port( port ),
			_topic( topic ),
			_ipBuffer( 32 ),
			_topicBuffer( 64 )
		{
			updateBuffers();
		}

		ZMQIn::~ZMQIn()
		{
			stop();
		}

		void ZMQIn::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool ZMQIn::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( _impl )
			{
				if( ImGui::Button( "stop" ) )
					stop();
			}
			else
			{
				if( ImGui::Button( "start" ) )
					start();
			}

			{
				ScopedImGuiDisable disable( _impl ? true : false );

				if( ImGui::InputText( "ip", &_ipBuffer[0], _ipBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					_ip = trim( &_ipBuffer[0] );

					updateBuffers();
				}

				int p = _port;
				if( ImGui::InputInt( "port", &p ) )
					_port = p;

				if( ImGui::InputText( "topic", &_topicBuffer[0], _topicBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					_topic = trim( &_topicBuffer[0] );

					updateBuffers();
				}
			}

			return true;
		}
#endif

		//TODO: when dealing with multiple inputs, this does not really make sense
		// think of a better way to do this
		bool ZMQIn::process()
		{
			if( _impl )
			{
				std::vector<SampleFrame*> frames;

				_impl->fetchFrames( frames );

				if( frames.size() )
				{
					//TODO: buffer -- for now, we just use the most recent one
					SampleFrame* ret = frames.back();

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

		bool ZMQIn::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			bool connected = false;
			load<bool>( j, "connected", connected );
			load<std::string>( j, "ip", _ip );
			load<uint16_t>( j, "port", _port );
			load<std::string>( j, "topic", _topic );

			updateBuffers();

			if( connected )
				start();

			return ret;
		}

		bool ZMQIn::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "connected", _impl ? true : false );
			save( j, "ip", _ip );
			save( j, "port", _port );
			save( j, "topic", _topic );

			return ret;
		}

		void ZMQIn::start()
		{
			stop();

			_impl = new ZMQSourceImpl();
			if( !_impl->run( _ip, _port, _topic ) )
			{
				std::cerr << "<error> failed to run ZMQ" << std::endl;
				stop();
			}
		}

		void ZMQIn::stop()
		{
			if( _impl )
			{
				_impl->close();
				safeDelete( _impl );
			}
		}

		void ZMQIn::updateBuffers()
		{
			strncpy( &_ipBuffer[0], _ip.c_str(), _ip.size() + 1 );
			strncpy( &_topicBuffer[0], _topic.c_str(), _topic.size() + 1 );
		}














		ZMQOut::ZMQOut( unsigned short port, const std::string& topic ) :
			Op(),
			_socket( nullptr ),
			_port( port ),
			_topic( topic ),
			_topicBuffer( 64 )
		{
			updateBuffers();
		}

		ZMQOut::~ZMQOut()
		{
			close();
		}

		bool ZMQOut::process()
		{
			SampleFrame* sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				if( _socket )
				{
					ZMQHdr hdr = {
						(uint16_t) sf->width(),
						(uint16_t) sf->height(),
						(uint16_t) sf->depth(),
						sf->timeStamp()
					};

					std::string t( "t:" + _topic );

					zmq_send( _socket, t.c_str(), t.length() + 1, ZMQ_SNDMORE );
					zmq_send( _socket, &hdr, sizeof( ZMQHdr ), ZMQ_SNDMORE );
					zmq_send( _socket, sf->values(), sf->size() * sizeof( float ), 0 );
				}

				drawFrame( sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool ZMQOut::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( !_socket )
			{
				if( ImGui::Button( "open" ) )
					open();
			}
			else
			{
				if( ImGui::Button( "close" ) )
					close();
			}

			{
				ScopedImGuiDisable disable( _socket ? true : false );

				int p = _port;
				if( ImGui::InputInt( "port", &p ) )
					_port = p;

				if( ImGui::InputText( "topic", &_topicBuffer[0], _topicBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					_topic = trim( &_topicBuffer[0] );

					updateBuffers();
				}
			}

			return true;
		}
#endif

		void ZMQOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

		bool ZMQOut::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			close();

			bool connected = false;

			load<bool>( j, "connected", connected );
			load<uint16_t>( j, "port", _port );
			load<std::string>( j, "topic", _topic );

			updateBuffers();

			if( connected )
				open();

			return ret;
		}

		bool ZMQOut::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "connected", _socket ? true : false );
			save( j, "port", _port );
			save( j, "topic", _topic );

			return ret;
		}

		bool ZMQOut::open()
		{
			close();

			auto ctxt = ZMQSingleton::get();
			if( !ctxt || !ctxt->ctx() )
			{
				std::cerr << "<error> zmqContext not initialized!" << std::endl;
				return false;
			}

			std::cout << "opening ZMQ socket" << std::endl;

			_socket = zmq_socket( ctxt->ctx(), ZMQ_PUB );
			if( !_socket )
			{
				std::cerr << "<error> creating socket failed: " << zmq_strerror( zmq_errno() ) << std::endl;
				return false;
			}

			std::string url( "tcp://*:" );
			url.append( toString( _port ) );

			int rc = zmq_bind( _socket, url.c_str() );
			if( rc < 0 )
			{
				std::cerr << "<error> failed to create zmq socket: " << zmq_strerror( zmq_errno() ) << std::endl;

				close();
				return false;
			}

			return true;
		}

		void ZMQOut::close()
		{
			if( _socket )
			{
				std::cout << "closing ZMQ socket" << std::endl;

				zmq_close( _socket );
				_socket = nullptr;
			}
		}

		void ZMQOut::updateBuffers()
		{
			strncpy( &_topicBuffer[0], _topic.c_str(), _topic.size() + 1 );
		}
	}
}