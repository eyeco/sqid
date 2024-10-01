/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <config.h>

#include <processing/op.h>

#include <fileIO/json.h>

namespace sqid
{
	namespace Plugins
	{
		class MQTTIn : public Op
		{
		private:
			int _handle;

			std::string _id;

			std::string _address;
			unsigned short _port;
			bool _cleanSession;
			int _keepAlive;

			std::string _topic;
			int _qos;

			bool _tryReconnect;
			int _reconnectTimeout;

			std::vector<char> _inputBufferID;
			std::vector<char> _inputBufferAddress;
			std::vector<char> _inputBufferTopic;

			void updateBuffers();

			bool open();
			void close();

		protected:
			virtual bool process();

		public:
			MQTTIn( const std::string& address = "localhost", unsigned short port = 1883, const std::string& topic = "sqid/data", int qos = 1, bool cleanSession = true, int keepAlive = 60 );
			virtual ~MQTTIn();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			DECLARE_OP_DESC;
		};

		class MQTTOut : public Op
		{
		private:
			static const size_t PACKAGE_MAX_SAMPLES = 1600;

			int _handle;

			std::string _id;

			std::string _address;
			unsigned short _port;
			bool _cleanSession;
			int _keepAlive;

			std::string _topic;
			int _qos;
			bool _retain;

			bool _tryReconnect;
			int _reconnectTimeout;

			std::vector<char> _inputBufferID;
			std::vector<char> _inputBufferAddress;
			std::vector<char> _inputBufferTopic;

			void updateBuffers();

			bool open();
			void close();

		protected:
			virtual bool process();

		public:
			explicit MQTTOut( const std::string& address = "localhost", unsigned short port = 1883, const std::string& topic = "sqid/data", int qos = 0, bool retain = false, bool cleanSession = true, int keepAlive = 60 );
			virtual ~MQTTOut();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			DECLARE_OP_DESC;
		};
	}
}
