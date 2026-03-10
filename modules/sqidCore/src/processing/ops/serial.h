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

#include "../../serialPort.h"

namespace sqid
{
	namespace Serial
	{
		//deprecated -> use COM/serial interface in combination with Sink op
		class SerialOut : public Op
		{
		private:

			bool _sending;

			std::string _portName;
			unsigned int _baud;
			//unsigned int _queueSize;

			unsigned char _deviceID;
			unsigned char _sensorID;

			SerialPort _port;

			unsigned int _dropdownSelected;
			unsigned int _baudUI;

			bool open();
			void close();

			//void updateBuffers();

		protected:
			virtual bool process();

		public:
			explicit SerialOut();
			virtual ~SerialOut();

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