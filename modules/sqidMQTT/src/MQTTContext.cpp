/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#include "MQTTContext.h"

#include <app.h>
#include <sampleFrame.h>

#include <mosquitto.h>

DEFINE_SINGLETON( sqid::Plugins::MQTTContext )

namespace sqid
{
	namespace Plugins
	{
		//TODO: use async functions for connect
// also look up the following in the docu
// - mosquitto_connect_async
// - mosquitto_reconnect_async
// have to use threaded interface mosquitto_loop_start()
// https://mosquitto.org/api/files/mosquitto-h.html

		MQTTPeer::MQTTPeer( int handle, bool tryReconnect, int reconnectTimeout, bool verbose ) :
			_handle( handle ),
			_host( "" ),
			_port( 1883 ),
			_keepAlive( 60 ),
			_tryReconnect( tryReconnect ),
			_reconnectTimeout( reconnectTimeout ),
			_reconnectRefTime( 0.0f ),
			_mosq( nullptr ),
			_connected( false ),
			_offline( false ),
			_isAbandoned( false ),
			_verbose( verbose )
		{}

		MQTTPeer::~MQTTPeer()
		{
			if( _mosq )
			{
				mosquitto_destroy( _mosq );
				_mosq = nullptr;
			}
		}

		void MQTTPeer::onConnect( struct mosquitto* mosq, void* obj, int reasonCode )
		{
			MQTTPeer* peer = reinterpret_cast<MQTTPeer*>( obj );
			if( mosq != peer->getMosquitto() )
				std::cerr << "<error> mosquitto handles differ!" << std::endl;
			peer->onConnect( reasonCode );
		}

		void MQTTPeer::onDisconnect( struct mosquitto* mosq, void* obj, int reasonCode )
		{
			MQTTPeer* peer = reinterpret_cast<MQTTPeer*>( obj );
			if( mosq != peer->getMosquitto() )
				std::cerr << "<error> mosquitto handles differ!" << std::endl;
			peer->onDisconnect( reasonCode );
		}

		void MQTTPeer::onConnect( int reasonCode )
		{
			std::cout << _id << " connected: " << mosquitto_connack_string( reasonCode ) << std::endl;
			if( reasonCode != MOSQ_ERR_SUCCESS )
			{
				std::cerr << "<error> connection issue, disconnecting MQTT publisher" << _id << std::endl;

				//TODO: not sure if onDisconnect will still be called even if something went wrong earlier... 
				// if no, we have to set _offline = false here so instance will be deleted in main loop 
				// -> doublecheck!!!
				mosquitto_disconnect( _mosq );
				_connected = false;
			}
			else
				_connected = true;
		}

		void MQTTPeer::onDisconnect( int reasonCode )
		{
			if( reasonCode != MOSQ_ERR_SUCCESS )
				std::cerr << "<warning> unexpectedly disconnected MQTT publisher " << _id << std::endl;
			else
				std::cout << _id << " disconnected: " << mosquitto_connack_string( reasonCode ) << std::endl;

			_connected = false;
			if( _tryReconnect )
				std::cout << "will try to reconnect in " << _reconnectTimeout << " seconds" << std::endl;
			else
			{
				_offline = true; //mark for deletion

				if( _mosq )
				{
					mosquitto_destroy( _mosq );
					_mosq = nullptr;
				}
			}

			_reconnectRefTime = getAppTime();
		}

		bool MQTTPeer::init( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive )
		{
			_host = std::string( host );
			_port = port;
			_keepAlive = keepAlive;

			_mosq = mosquitto_new( id.size() ? id.c_str() : nullptr, true, this );
			if( _mosq == NULL )
			{
				std::cerr << "<error> out of memory." << std::endl;

				_offline = true; //mark for deletion

				return false;
			}

			_id = id.size() ? id : "<anonymous>";

			mosquitto_connect_callback_set( _mosq, MQTTPeer::onConnect );
			mosquitto_disconnect_callback_set( _mosq, MQTTPeer::onDisconnect );

			return true;
		}

		bool MQTTPeer::connect()
		{
			std::cout << "connecting " << _id << " to " << _host << ":" << _port << std::endl;

			int rc = mosquitto_connect( _mosq, _host.c_str(), _port, _keepAlive );
			if( rc != MOSQ_ERR_SUCCESS )
			{
				std::cerr << "<error> failed to connect " << _id << " to " << _host << ":" << _port << ": " << mosquitto_strerror( rc ) << std::endl;

				if( _tryReconnect )
				{
					_reconnectRefTime = getAppTime();
					std::cout << "will try to reconnect in " << _reconnectTimeout << " seconds" << std::endl;
				}
				else
				{
					mosquitto_destroy( _mosq );
					_mosq = nullptr;

					_offline = true; //mark for deletion
				}

				return false;
			}

			return true;
		}

		bool MQTTPeer::disconnect()
		{
			_tryReconnect = false;

			if( !_connected )
				return false;

			if( !_mosq )
			{
				std::cerr << "<error> " << _id << " handle already destroyed" << std::endl;
				return false;
			}

			mosquitto_disconnect( _mosq );

			return true;
		}

		float MQTTPeer::tryReconnectNow() const
		{
			return _tryReconnect && ( getAppTime() - _reconnectRefTime >= _reconnectTimeout );
		}




		void MQTTSub::onSubscribe( struct mosquitto* mosq, void* obj, int mid, int qosCount, const int* qosGranted )
		{
			MQTTSub* sub = reinterpret_cast<MQTTSub*>( obj );
			if( mosq != sub->getMosquitto() )
				std::cerr << "<error> mosquitto handles differ!" << std::endl;
			sub->onSubscribe( mid, qosCount, qosGranted );
		}

		void MQTTSub::onMessage( struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg )
		{
			MQTTSub* sub = reinterpret_cast<MQTTSub*>( obj );
			if( mosq != sub->getMosquitto() )
				std::cerr << "<error> mosquitto handles differ!" << std::endl;
			sub->onMessage( msg );
		}

		void MQTTSub::onSubscribe( int mid, int qosCount, const int* qosGranted )
		{
			int i;
			bool haveSubscription = false;

			for( i = 0; i < qosCount; i++ )
			{
				std::cout << "subscription " << i << ": granted qos = " << qosGranted[i] << std::endl;
				if( qosGranted[i] <= 2 )
					haveSubscription = true;
			}
			if( haveSubscription == false )
			{
				//broker rejected all of our requested subscriptions
				std::cerr << "<error> All subscriptions rejected." << std::endl;
				disconnect();
			}
		}

		void MQTTSub::onMessage( const struct mosquitto_message* msg )
		{
			if( _verbose )
				std::cout << "received mid " << msg->mid << " topic \"" << msg->topic << "\" qos:" << msg->qos << " (" << msg->payloadlen << " bytes)" << std::endl;

			if( msg->payloadlen < sizeof( MQTTHdr ) )
			{
				std::cerr << "<error> invalid payload length" << std::endl;
				return;
			}

			const MQTTHdr* hdr = reinterpret_cast<const MQTTHdr*>( msg->payload );
			size_t samples = hdr->w * hdr->h * hdr->d;

			//sanity check
			if( msg->payloadlen - sizeof( MQTTHdr ) != samples * sizeof( float ) )
			{
				std::cerr << "<error> unexpected data length (" << msg->payloadlen - sizeof( MQTTHdr ) << ", expected " << samples * sizeof( float ) << ")" << std::endl;
				return;
			}

			const float* f = reinterpret_cast<const float*>( (uint8_t*) ( msg->payload ) + sizeof( MQTTHdr ) );
			SampleFrame* sf = new SampleFrame( hdr->w, hdr->h, f, hdr->ts, hdr->d );

			{
				std::lock_guard lock( _dataMutex );

				_buffer.push_back( sf );

				while( _buffer.size() > _bufferMaxSize )
				{
					safeDelete( _buffer.front() );
					_buffer.pop_front();
				}
			}
		}

		void MQTTSub::onConnect( int reasonCode )
		{
			MQTTPeer::onConnect( reasonCode );

			//subscribing in onConnect callback means that if the connection drops and is automatically 
			// resumed by the client, then the subscriptions will be recreated when the client reconnects.
			int rc = mosquitto_subscribe( _mosq, nullptr, _topic.c_str(), _qos );
			if( rc != MOSQ_ERR_SUCCESS )
			{
				std::cerr << "<error> subscribing to topic " << _topic << " with qos = " << _qos << " failed: " << mosquitto_strerror( rc ) << std::endl;
				disconnect();
			}
		}

		MQTTSub::MQTTSub( int handle, const std::string& topic, int qos, bool tryReconnect, int reconnectTimeout, size_t bufferMaxSize ) :
			MQTTPeer( handle, tryReconnect, reconnectTimeout ),
			_topic( topic ), _qos( qos ),
			_bufferMaxSize( bufferMaxSize )
		{}

		MQTTSub::~MQTTSub()
		{
			std::lock_guard lock( _dataMutex );

			for( auto it : _buffer )
				safeDelete( it );
			_buffer.clear();
		}

		bool MQTTSub::init( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive )
		{
			bool ret = MQTTPeer::init( id, cleanSession, host, port, keepAlive );

			if( ret )
			{
				mosquitto_subscribe_callback_set( _mosq, MQTTSub::onSubscribe );
				mosquitto_message_callback_set( _mosq, MQTTSub::onMessage );
			}

			return ret;
		}

		void MQTTSub::fetch( std::vector<SampleFrame*>& frames )
		{
			{
				std::lock_guard lock( _dataMutex );

				for( auto it : _buffer )
					frames.push_back( it );

				_buffer.clear();
			}
		}





		MQTTPub::MQTTPub( int handle, bool tryReconnect, int reconnectTimeout ) :
			MQTTPeer( handle, tryReconnect, reconnectTimeout )
		{}

		MQTTPub::~MQTTPub()
		{}

		void MQTTPub::onPublish( struct mosquitto* mosq, void* obj, int mid )
		{
			MQTTPub* pub = reinterpret_cast<MQTTPub*>( obj );
			if( mosq != pub->getMosquitto() )
				std::cerr << "<error> mosquitto handles differ!" << std::endl;
			pub->onPublish( mid );
		}

		void MQTTPub::onPublish( int mid )
		{
			if( _verbose )
				std::cout << "message with mID " << mid << " has been published by " << _id << std::endl;
		}

		bool MQTTPub::init( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive )
		{
			bool ret = MQTTPeer::init( id, cleanSession, host, port, keepAlive );

			if( ret )
			{
				mosquitto_publish_callback_set( _mosq, MQTTPub::onPublish );
			}

			return ret;
		}

		bool MQTTPub::publish( const std::string& topic, const SampleFrame* sf, int qos, bool retain )
		{
			if( !_connected || !_mosq )
				return false;

			if( !topic.size() )
				return false;

			if( !sf )
				return false;

			size_t headerBytes = sizeof( MQTTHdr );
			size_t payloadBytes = sf->size() * sizeof( float );
			size_t bytes = headerBytes + payloadBytes;
			if( _buffer.size() < bytes )
				_buffer.resize( nextPo2( bytes ) );
			MQTTHdr* hdr = reinterpret_cast<MQTTHdr*>( &_buffer[0] );
			hdr->w = sf->width();
			hdr->h = sf->height();
			hdr->d = sf->depth();
			hdr->ts = sf->timeStamp();
			if( payloadBytes )
				memcpy( &_buffer[headerBytes], sf->values(), payloadBytes );

			int mid = 0;
			int ret = mosquitto_publish( _mosq, &mid, topic.c_str(), bytes, &_buffer[0], qos, retain );

			if( ret != MOSQ_ERR_SUCCESS )
			{
				std::cerr << "<error> failed publishing mID " << mid << ": " << mosquitto_strerror( ret ) << " (" << ret << ")" << std::endl;
				return false;
			}

			return true;
		}






		MQTTContext::MQTTContext() :
			_initialized( false ),
			_keepRunning( true ),
			_runThread( nullptr )
		{
			init();

			_runThread = new std::thread( &MQTTContext::run, this );
		}

		MQTTContext::~MQTTContext()
		{
			shutdown();
		}

		void MQTTContext::insert( MQTTPeer* peer )
		{
			std::cout << "inserting peer " << peer->getID() << " into list" << std::endl;

			if( _peers.find( peer->getHandle() ) != _peers.end() )
				std::cerr << "<warning> peer " << peer->getID() << " already inserted!" << std::endl;
			else
				_peers.insert( std::make_pair( peer->getHandle(), peer ) );
		}

		void MQTTContext::remove( MQTTPeer* peer )
		{
			if( !peer )
				return;

			std::cout << "removing peer " << peer->getID() << " into list" << std::endl;

			auto it = _peers.find( peer->getHandle() );
			if( it == _peers.end() )
				std::cerr << "<error> peer " << peer->getID() << " not found in list!" << std::endl;
			else
				_peers.erase( it );
		}

		bool MQTTContext::init()
		{
			if( !_initialized )
			{
				int major = 0;
				int minor = 0;
				int revision = 0;

				mosquitto_lib_version( &major, &minor, &revision );

				std::cout << "initializing Mosqitto "
					<< major << "." << minor << "." << revision << " (compiled with version "
					<< LIBMOSQUITTO_MAJOR << "." << LIBMOSQUITTO_MINOR << "." << LIBMOSQUITTO_REVISION << ")" << std::endl;

				int ret = MOSQ_ERR_SUCCESS;
				if( ( ret = mosquitto_lib_init() ) == MOSQ_ERR_SUCCESS )
				{
					_initialized = true;
					std::cout << "success" << std::endl;
				}
				else
				{
					std::cerr << "initializing Mosquitto failed (error code #" << ret << ")" << std::endl;
					return false;
				}
			}

			return true;
		}

		void MQTTContext::shutdown()
		{
			_keepRunning = false;
			if( _runThread )
			{
				std::cout << "stopping Mosqitto thread" << std::endl;
				_runThread->join();
				safeDelete( _runThread );
			}

			if( _initialized )
			{
				std::cout << "shutting down Mosqitto lib" << std::endl;
				mosquitto_lib_cleanup();
			}

			//NOTE: after shutting down, it's hopefully safe to assume that no more
			// callbacks are called and we can just go ahead and delete everything 
			// that's left? possible source of error, still feeling a bit paranoid.
			for( auto it : _peers )
				safeDelete( it.second );
			_peers.clear();
		}

		void MQTTContext::run()
		{
			while( _keepRunning )
			{
				{
					std::lock_guard lock( _peerMutex );

					for( auto it : _peers )
						mosquitto_loop( it.second->getMosquitto(), 0, 1 );

					auto it = _peers.begin();
					while( it != _peers.end() )
					{
						MQTTPeer* peer = it->second;
						if( !peer->getConnected() )
						{
							if( peer->tryReconnectNow() )
								if( !peer->connect() )
									std::cerr << "<error> reconnection attempt failed" << std::endl;
						}

						if( peer->getOffline() && peer->getIsAbandoned() )
						{
							std::cout << "deleting disconnected peer " << peer->getID() << std::endl;

							safeDelete( peer );
							it = _peers.erase( it );
						}
						else
							++it;
					}
				}

				Sleep( 1 );
			}
		}

		int MQTTContext::createSubscriber( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive, const std::string& topic, int qos, bool tryReconnect, int reconnectTimeout )
		{
			if( !host )
				return 0;

			MQTTSub* sub = new MQTTSub( ++HandleCntr, topic, qos, tryReconnect, reconnectTimeout );

			{
				std::lock_guard lock( _peerMutex );

				if( !sub->init( id, cleanSession, host, port, keepAlive ) )
				{
					safeDelete( sub );
					return 0;
				}

				if( !sub->connect() && !tryReconnect )
				{
					safeDelete( sub );
					return 0;
				}

				insert( sub );
			}

			return sub->getHandle();
		}

		int MQTTContext::createPublisher( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive, bool tryReconnect, int reconnectTimeout )
		{
			if( !host )
				return 0;

			MQTTPub* pub = new MQTTPub( ++HandleCntr, tryReconnect, reconnectTimeout );

			{
				std::lock_guard lock( _peerMutex );

				if( !pub->init( id, cleanSession, host, port, keepAlive ) )
				{
					safeDelete( pub );
					return 0;
				}

				if( !pub->connect() && !tryReconnect )
				{
					safeDelete( pub );
					return 0;
				}

				insert( pub );
			}

			return pub->getHandle();
		}

		bool MQTTContext::destroy( int handle )
		{
			if( !handle )
				return false;

			{
				std::lock_guard lock( _peerMutex );

				auto it = _peers.find( handle );
				if( it == _peers.end() )
					return false;

				it->second->disconnect();
				it->second->markAbandoned();
			}

			return true;
		}

		//returns false if (and only if!) peer handle should be no longer used
		bool MQTTContext::publish( const SampleFrame* sf, int handle, const std::string& topic, int qos, bool retain )
		{
			std::lock_guard lock( _peerMutex );

			auto it = _peers.find( handle );
			if( it == _peers.end() )
			{
				std::cerr << "<error> publisher not found in peer list" << std::endl;
				return false;
			}

			MQTTPub* pub = dynamic_cast<MQTTPub*>( it->second );
			if( pub )
			{
				if( pub->getOffline() )
				{
					if( !pub->getAutoReconnect() )
					{
						pub->markAbandoned();
						return false;
					}
				}

				if( pub->getConnected() && !pub->publish( topic, sf, qos, retain ) )
					std::cerr << "<error> publishing data failed" << std::endl;
			}
			else
				std::cerr << "<warning> peer supposed to be a publisher, but apparently is not" << std::endl;

			return true;
		}

		//returns false if (and only if!) peer handle should be no longer used
		bool MQTTContext::fetchFrames( int handle, std::vector<SampleFrame*>& frames )
		{
			if( frames.size() )
				std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory here!" << std::endl;

			std::lock_guard lock( _peerMutex );

			auto it = _peers.find( handle );
			if( it == _peers.end() )
			{
				std::cerr << "<error> publisher not found in peer list" << std::endl;
				return false;
			}

			MQTTSub* sub = dynamic_cast<MQTTSub*>( it->second );
			if( sub )
			{
				if( sub->getOffline() )
				{
					if( !sub->getAutoReconnect() )
					{
						sub->markAbandoned();
						return false;
					}
				}

				sub->fetch( frames );
			}
			else
				std::cerr << "<warning> peer supposed to be a subscriber, but apparently is not" << std::endl;

			return true;
		}
	}
}
