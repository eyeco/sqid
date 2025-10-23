/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <singleton.h>

#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

struct mosquitto;
struct mosquitto_message;

namespace sqid
{
	class SampleFrame;

	namespace Plugins
	{
		//TODO: use async functions for connect
		// also look up the following in the docu
		// - mosquitto_connect_async
		// - mosquitto_reconnect_async
		// have to use threaded interface mosquitto_loop_start()
		// https://mosquitto.org/api/files/mosquitto-h.html

#pragma pack(push)
#pragma pack(1)
		struct MQTTHdr
		{
			unsigned short w;
			unsigned short h;
			unsigned short d;

			unsigned int ts;
		};
#pragma pack(pop)

		class MQTTPeer
		{
		private:
			static void onConnect( struct mosquitto* mosq, void* obj, int reasonCode );
			static void onDisconnect( struct mosquitto* mosq, void* obj, int reasonCode );

		protected:
			int _handle;

			std::string _host;
			int _port;
			int _keepAlive;

			bool _tryReconnect;
			int _reconnectTimeout;
			float _reconnectRefTime;

			mosquitto* _mosq;
			std::string _id;
			bool _connected;
			bool _offline;
			bool _isAbandoned;

			bool _verbose;

			virtual void onConnect( int reasonCode );
			virtual void onDisconnect( int reasonCode );

			virtual bool init( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive );

			MQTTPeer( int handle, bool tryReconnect, int reconnectTimeout, bool verbose = false );

		public:
			virtual ~MQTTPeer();

			bool connect();
			bool disconnect();

			int getHandle() const { return _handle; }

			struct mosquitto* getMosquitto() const { return _mosq; }
			const std::string& getID() const { return _id; }

			bool getConnected() const { return _connected; }

			//returns true if no longer connected, but was either connected before or connection attempt failed
			bool getOffline() const { return _offline; }

			bool getAutoReconnect() const { return _tryReconnect; }

			bool getIsAbandoned() const { return _isAbandoned; }
			void markAbandoned() { _isAbandoned = true; }

			float tryReconnectNow() const;
		};

		class MQTTSub : public MQTTPeer
		{
		private:
			std::string _topic;
			int _qos;

			size_t _bufferMaxSize;
			std::list<SampleFrame*> _buffer;

			std::mutex _dataMutex;

			static void onSubscribe( struct mosquitto* mosq, void* obj, int mid, int qosCount, const int* qosGranted );
			static void onMessage( struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg );

			void onSubscribe( int mid, int qosCount, const int* qosGranted );
			void onMessage( const struct mosquitto_message* msg );

		protected:
			virtual void onConnect( int reasonCode );

		public:
			MQTTSub( int handle, const std::string& topic, int qos, bool tryReconnect, int reconnectTimeout, size_t bufferMaxSize = 128 );
			virtual ~MQTTSub();

			virtual bool init( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive );

			void fetch( std::vector<SampleFrame*>& frames );
		};

		class MQTTPub : public MQTTPeer
		{
		private:
			std::vector<uint8_t> _buffer;

			static void onPublish( struct mosquitto* mosq, void* obj, int mid );

			void onPublish( int mid );

		public:
			MQTTPub( int handle, bool tryReconnect, int reconnectTimeout );
			virtual ~MQTTPub();

			virtual bool init( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive );
			bool publish( const std::string& topic, const SampleFrame* sf, int qos, bool retain );
		};

		class MQTTContext
		{
		private:
			static int HandleCntr;

			bool _initialized;
			bool _keepRunning;

			std::thread* _runThread;
			std::mutex _peerMutex;

			std::map<int, MQTTPeer*> _peers;

			void insert( MQTTPeer* peer );
			void remove( MQTTPeer* peer );

			bool init();
			void shutdown();

			void run();

		public:
			MQTTContext();
			~MQTTContext();

			int createSubscriber( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive, const std::string& topic, int qos, bool tryReconnect, int reconnectTimeout );
			int createPublisher( const std::string& id, bool cleanSession, const char* host, int port, int keepAlive, bool tryReconnect, int reconnectTimeout );

			bool destroy( int handle );

			//returns false if (and only if!) peer handle should be no longer used
			bool publish( const SampleFrame* sf, int handle, const std::string& topic, int qos, bool retain );

			//returns false if (and only if!) peer handle should be no longer used
			bool fetchFrames( int handle, std::vector<SampleFrame*>& frames );
		};
	}
}

DECLARE_SINGLETON( sqid::Plugins::MQTTContext, MQTTSingleton )