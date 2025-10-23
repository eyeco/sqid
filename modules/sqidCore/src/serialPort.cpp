/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "serialPort.h"

#include <serial/serial.h>

#include <iostream>

namespace sqid
{
	namespace Internal
	{
		class SerialPortImpl
		{
		private:
			static bool _initialized;

			static std::vector<serial::PortInfo> _knownDevices;
			static std::vector<std::string> _knownPorts;

			serial::Serial _serialPort;

		public:
			SerialPortImpl()
			{
				serial::Timeout to = serial::Timeout::simpleTimeout( 1000 );
				_serialPort.setTimeout( to );
			}

			~SerialPortImpl()
			{
				close();
			}

			bool open( const std::string &port, unsigned int baud )
			{
				close();

				std::string portName( port );

				//this is required for COM ports with id greater than 9
				if( portName.find( "\\\\.\\" ) == std::string::npos )
					portName.insert( 0, "\\\\.\\" );
				std::cout << "opening \"" << portName << "\" @ " << baud << " baud" << std::endl;

				_serialPort.setPort( port );
				_serialPort.setBaudrate( baud );

				try
				{
					_serialPort.open();
				}
				catch( std::exception &e )
				{
					std::cerr << "<error> unable to connect to com port " << port << " with baud rate " << baud << ": " << e.what() << std::endl;

					return false;
				}

				return true;
			}

			void close()
			{
				_serialPort.close();
			}

			bool isOpen() const
			{
				return _serialPort.isOpen();
			}

			size_t available()
			{
				return _serialPort.available();
			}

			size_t read( uint8_t *buffer, size_t bufferSize )
			{
				try
				{
					return _serialPort.read( buffer, bufferSize );
				}
				catch( serial::SerialException &se )
				{
					std::cerr << "<error> caught exception while reading from serial port: " << se.what() << std::endl;
					return -1;
				}
			}

			size_t write( const uint8_t *data, size_t size )
			{
				return _serialPort.write( data, size );
			}


			static bool init()
			{
				if( !_initialized )
				{
					rescan();
					_initialized = true;
				}

				return _initialized;
			}

			static bool isInitialized()
			{
				return _initialized;
			}

			static void rescan()
			{
				_knownDevices.clear();

				std::cout << "scanning for COM devices..." << std::endl;

				_knownDevices = serial::list_ports();

				_knownPorts.clear();
				for( const auto& d : _knownDevices )
					_knownPorts.push_back( d.port );

				std::cout << "found " << _knownDevices.size() << " COM device(s)" << std::endl;
			}

			static void enumerate()
			{
				int cntr = 0;
				for( const auto& d : _knownDevices )
				{
					std::cout << "  device #" << cntr++ << std::endl;
					std::cout << "    port: " << d.port << std::endl;
					std::cout << "    description: " << d.description << std::endl;
					std::cout << "    HW ID: " << d.hardware_id << std::endl;
				}
			}

			static const std::vector<serial::PortInfo> &getKnownDevices() { return _knownDevices; }
			static const std::vector<std::string> &getKnownPorts() { return _knownPorts; }
		};

		bool SerialPortImpl::_initialized = false;
		std::vector<serial::PortInfo> SerialPortImpl::_knownDevices;
		std::vector<std::string> SerialPortImpl::_knownPorts;
	}




	unsigned short getPortNr( const std::string &portName )
	{
		std::string s( portName );
		std::transform( s.begin(), s.end(), s.begin(), tolower );
		size_t pos = s.find( "com" );
		if( pos != std::string::npos )
			return atoi( s.c_str() + pos + 3 );

		std::cerr << "could not locate port nr in string" << std::endl;
		return 0xffff;
	}



	SerialPort::SerialPort() :
		_impl( new Internal::SerialPortImpl() )
	{}

	SerialPort::~SerialPort()
	{
		safeDelete( _impl );
	}

	bool SerialPort::open( const std::string &port, unsigned int baud )
	{
		return _impl->open( port, baud );
	}

	void SerialPort::close()
	{
		_impl->close();
	}

	bool SerialPort::isOpen() const
	{
		return _impl->isOpen();
	}

	size_t SerialPort::available()
	{
		return _impl->available();
	}

	size_t SerialPort::read( uint8_t *buffer, size_t bufferSize )
	{
		return _impl->read( buffer, bufferSize );
	}

	size_t SerialPort::write( const uint8_t *data, size_t size )
	{
		return _impl->write( data, size );
	}

	bool SerialPort::init()
	{
		return Internal::SerialPortImpl::init();
	}

	bool SerialPort::isInitialized()
	{
		return Internal::SerialPortImpl::isInitialized();
	}

	void SerialPort::rescan()
	{
		Internal::SerialPortImpl::rescan();
	}

	void SerialPort::enumerate()
	{
		Internal::SerialPortImpl::enumerate();
	}

	const std::vector<std::string> &SerialPort::getKnownPorts()
	{
		return Internal::SerialPortImpl::getKnownPorts();
	}

}