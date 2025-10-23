/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <serial/serial.h>

#include <iostream>
#include <exception>

#include <conio.h>
#include <signal.h>

#include <Windows.h>

#include <cxxopts.hpp>

std::string port;
unsigned int baud = 115200;

int timeout = 1000;

bool done = false;
bool doList = false;

bool verbose = false;

template<typename T>
static std::string toString( const T &t )
{
	std::stringstream sstr;
	sstr << t;

	return sstr.str();
}

template<>
static std::string toString<bool>( const bool &t )
{
	return std::string( t ? "true" : "false" );
}

static bool readPA( int argc, char **argv )
{
	//generic options: 
	cxxopts::Options options( argv[0], " command line options" );
	options
		.positional_help( "[optional args]" )
		.show_positional_help();

	options.add_options()
		( "h,help", "print help" )
		( "p,port", "full name of serial port", cxxopts::value<std::string>() )
		( "b,baud", "baud rate", cxxopts::value<unsigned int>()->default_value( toString( baud ) ) )
		( "l,list", "list all available ports" )
		( "t,timeout", "serial R/W timeout in millisecods", cxxopts::value<unsigned int>()->default_value( toString( timeout ) ) )
		( "v,verbose", "verbose output" )
		;

	//positional options:
	options.parse_positional( { "p" } );

	try
	{
		cxxopts::ParseResult result = options.parse( argc, argv );

		//generic options
		if( result.count( "help" ) )
		{
			std::cout << options.help( { "" } ) << std::endl;
			return false;
		}

		if( result.count( "verbose" ) )
			verbose = true;
		else
			verbose = false;

		if( result.count( "list" ) )
		{
			doList = true;
		}
		else
		{
			if( !result.count("port") )
				throw std::runtime_error( "port argument is missing" );
			port = result["port"].as<std::string>();
			timeout = result["timeout"].as<unsigned int>();
			baud = result["baud"].as<unsigned int>();
		}
	}
	catch( std::exception &e )
	{
		std::cerr << e.what() << std::endl;

		std::stringstream sstr;
		sstr << std::endl << "An error occured, processing program options: " << std::endl
			<< e.what() << std::endl;

		std::cout << options.help();

		throw std::runtime_error( sstr.str().c_str() );
	}

	return true;
}

void enumPorts()
{
	std::vector<serial::PortInfo> devices = serial::list_ports();

	std::vector<serial::PortInfo>::iterator iter = devices.begin();

	while( iter != devices.end() )
	{
		serial::PortInfo device = *iter++;

		printf( "(%s, %s, %s)\n", device.port.c_str(), device.description.c_str(),
			device.hardware_id.c_str() );
	}
}

void ctrlc( int sig )
{
	done = true;
}

void cleanup()
{
}

void __cdecl onExit()
{
	cleanup();
}

void __cdecl onTerminate()
{
	abort();
}

void __cdecl onUnexpected()
{
}


int main( int argc, char **argv )
{
	atexit( onExit );
	std::set_terminate( onTerminate );
	std::set_unexpected( onUnexpected );

	try
	{
		if( readPA( argc, argv ) )
		{
			if( doList )
				enumPorts();
			else
			{
				signal( SIGINT, ctrlc );

				std::cout << "opening serial port " << port << " with " << baud << " bd (timeout = " << timeout << " ms)" << std::endl;

#ifdef _WIN32
				if( port.find( "\\\\.\\" ) == std::string::npos )
				{
					port.insert( 0, "\\\\.\\" );
					std::cout << "NOTE: inserted Windows specific prefix, changed to " << port << std::endl;
				}
#endif

				serial::Serial serial;
				serial.setPort( port );
				serial.setBaudrate( baud );
				serial.setTimeout( serial::Timeout::simpleTimeout( timeout ) );
				serial.open();

				if( !serial.isOpen() )
				{
					std::cerr << "ERROR: failed to open serial port" << std::endl;
				}
				else
				{
					std::vector<uint8_t> buffer( 128 );
					std::cout << "starting to read serial port" << std::endl;
					while( !done )
					{
						serial.flush();

						if( !serial.available() )
						{
							if( verbose )
								std::cout << "no data available, waiting..." << std::endl;
							Sleep( 10 );
						}
						else
						{
							if( verbose )
								std::cout << "trying to read " << buffer.size() << " bytes..." << std::endl;
							size_t read = serial.read( &buffer[0], buffer.size() );
							if( verbose )
							{
								std::cout << "read " << read << " bytes: \"";
								std::cout.write( (char*)&buffer[0], read );
								std::cout << "\"" << std::endl;
							}
							else if( read )
								std::cout.write( (char*)&buffer[0], read );
						}
					}

					serial.close();
				}
			}
		}
	}
	catch( std::exception &e )
	{
		std::cerr << "caught exception: " << e.what() << std::endl;

#ifdef _DEBUG
		std::cout << "press ENTER" << std::endl;
		getchar();
#endif

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}