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

#include <processing/op.h>

namespace sqid
{
	namespace OSC
	{
		class OSCOut : public Op
		{
		public:
			enum OutType
			{
				OT_SAMPLEFRAME,
				OT_NATIVE,
				OT_NATIVE_CUSTOM,	//TODO: implement custom types (e.g., let the user specify type descriptor as string, for mixing multiple types in OSC package)

				OT_COUNT
			};

			enum NativeType
			{
				// basic OSC types
				NT_INT32,
				NT_FLOAT,
				//NT_STRING,
				//NT_BLOB,

				// extended OSC types
				NT_INT64,
				//NT_TIMETAG,
				NT_DOUBLE,
				//NT_SYMBOL,
				NT_CHAR,
				//NT_MIDI,
				//NT_TRUE,
				//NT_FALSE,
				//NT_NIL,
				//NT_INF,

				NT_COUNT
			};

		private:
			static const size_t PACKAGE_MAX_SAMPLES = 1600;

			bool _sending;

			Protocol _proto;

			std::string _ip;
			unsigned short _port;

			std::string _path;

			OutType _outType;
			NativeType _nativeType;

			std::vector<char> _inputBufferIP;
			std::vector<char> _inputBufferPort;
			std::vector<char> _inputBufferPath;

			void *_address;

			void updateBuffers();

			bool open();
			void close();

		protected:
			virtual bool process();

		public:
			explicit OSCOut( const std::string &ip = "127.0.0.1", unsigned short port = 6667, const std::string &oscPath = "/sqid" );
			virtual ~OSCOut();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
	}
}