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

#include <common.h>

namespace sqid
{
	namespace Internal
	{
		class SerialPortImpl;
	}

	unsigned short getPortNr( const std::string &portName );

	class SerialPort
	{
	private:

		Internal::SerialPortImpl *_impl;

	public:
		SerialPort();
		~SerialPort();

		bool open( const std::string &port, unsigned int baud );
		void close();

		bool isOpen() const;

		size_t available();
		size_t read( uint8_t *buffer, size_t bufferSize );
		size_t write( const uint8_t *data, size_t size );

		static bool init();
		static bool isInitialized();
		static void rescan();
		static void enumerate();
		static const std::vector<std::string> &getKnownPorts();
		static const std::vector<std::string> &getKnownPortNames();
	};
}