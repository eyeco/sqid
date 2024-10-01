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

// include nlohmann JSON if you plan to load/save your custom parameters
#include <fileIO/json.h>

namespace sqid
{
	namespace Plugins
	{
		class ZMQSourceImpl;

		class ZMQIn : public Op
		{
		private:
			ZMQSourceImpl* _impl;

			std::string _ip;
			unsigned short _port;

			std::string _topic;

			std::vector<char> _ipBuffer;
			std::vector<char> _topicBuffer;

			void updateBuffers();

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			ZMQIn( const std::string& ip = "localhost", unsigned short port = 5555, const std::string& topic = "sqid" );
			virtual ~ZMQIn();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			DECLARE_OP_DESC;
		};

		class ZMQOut : public Op
		{
		private:
			unsigned short _port;

			void* _socket;

			std::string _topic;
			std::vector<char> _topicBuffer;

			void updateBuffers();

			bool open();
			void close();

		protected:
			virtual bool process();

		public:
			explicit ZMQOut( unsigned short port = 5555, const std::string& topic = "sqid" );
			virtual ~ZMQOut();

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
