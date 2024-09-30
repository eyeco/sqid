/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/



#include <fstream>
#include <iostream>
#include <exception>

#include <conio.h>
#include <signal.h>
#include <Windows.h>

#include <lo/lo.h>

#include <nlohmann/json.hpp>

#include <cxxopts.hpp>

std::string configFile;

bool done = false;
bool verbose = false;
unsigned short oscPort = 0;
bool bundled = false;
lo_timetag tt_now;
lo_timetag tt_bundle;

lo_server server = nullptr;


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


struct Destination
{
public:
	Destination( const std::string &ip, unsigned short port, const std::string &filter ) :
		address( lo_address_new( ip.c_str(), toString( port ).c_str() ) ),
		filter( filter )
	{}

	~Destination()
	{
		if( address )
		{
			lo_address_free( address );
			address = nullptr;
		}
	}

	lo_address address;
	std::string filter;
};

std::vector<Destination*> destinations;


int bundleStartHandler( lo_timetag tt, void *user_data )
{
	if( tt.sec == LO_TT_IMMEDIATE.sec &&
		tt.frac == LO_TT_IMMEDIATE.frac )
	{
		lo_timetag_now( &tt_now );
		tt_bundle.sec = tt_now.sec;
		tt_bundle.frac = tt_now.frac;
	}
	else
	{
		tt_bundle.sec = tt.sec;
		tt_bundle.frac = tt.frac;
	}
	bundled = true;
	return 0;
}

int bundleEndHandler( void *user_data )
{
	bundled = false;
	return 0;
}

void errorHandler( int num, const char *msg, const char *where )
{
	if( msg && where )
		std::cerr << "<error> liblo server error " << num << " in path " << where << ": " << msg << std::endl;
	else if( msg )
		std::cerr << "<error> liblo server error " << num << ": " << msg << std::endl;
	else if( where )
		std::cerr << "<error> liblo server error " << num << " in path " << where << std::endl;
	else
		std::cerr << "<error> liblo server error " << num << std::endl;
}

int messageHandler( const char *path, const char *types, lo_arg ** argv,
	int argc, lo_message msg, void *user_data )
{
	try
	{
		if( verbose )
		{
			lo_address srcAddr = lo_message_get_source( msg );
			const char *srcHost = lo_address_get_hostname( srcAddr );
			const char *srcPort = lo_address_get_port( srcAddr );
			int srcProto = lo_address_get_protocol( srcAddr );
			const char *srcProtoStr = "UNKNOWN";
			switch( srcProto )
			{
			case LO_DEFAULT:
				srcProtoStr = "DEFAULT";
				break;
			case LO_UDP:
				srcProtoStr = "UDP";
				break;
			case LO_UNIX:
				srcProtoStr = "UNIX";
				break;
			case LO_TCP:
				srcProtoStr = "TCP";
				break;
			}

			lo_timetag tt;

			if( bundled )
				tt = tt_bundle;
			else
			{
				tt = lo_message_get_timestamp( msg );
				if( tt.sec == LO_TT_IMMEDIATE.sec &&
					tt.frac == LO_TT_IMMEDIATE.frac )
				{
					lo_timetag_now( &tt_now );
					tt = tt_now;
				}
			}

			std::cout << "<" << tt.sec << "." << tt.frac << "> src: " << srcHost << ":" << srcPort << " [" << srcProtoStr << "]; local port: " << oscPort << std::endl
				<< "\t" << path << std::endl;
		}

		for( auto &it : destinations )
		{
			if( !strncmp( path, it->filter.c_str(), it->filter.size() ) )
			{
				if( verbose )
					std::cout << " -> forwarding to " << lo_address_get_hostname( it->address ) << ":" << lo_address_get_port( it->address ) << std::endl;

				lo_send_message( it->address, path, msg );
			}
		}
	}
	catch( std::exception &e )
	{
		std::cout << "error while parsing message: " << path << ": " << e.what() << "\n";
	}

	return 0;
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
		( "v,verbose", "enable verbose output" )
		( "configFile", "location of config file, relative to working directory", cxxopts::value<std::string>() )
		;

	//positional options:
	options.parse_positional( { "configFile" } );

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

		if( !result.count( "configFile" ) )
			throw std::runtime_error( "config file not specified" );
		configFile = result["configFile"].as<std::string>();
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

void readConfigFile( const std::string &path )
{
	using json = nlohmann::json;

	std::ifstream i( path );

	if( !i.is_open() )
	{
		std::stringstream sstr;
		sstr << "failed to open config file " << path;
		throw std::runtime_error( sstr.str() );
	}

	json root;
	i >> root;

	oscPort = root["oscPort"].get<unsigned short>();

	auto dests = root["destinations"];
	if( !dests.is_null() )
	{
		for( auto it = dests.begin(); it != dests.end(); ++it )
		{
			destinations.push_back( new Destination( 
				it.value()["ip"].get<std::string>(),
				it.value()["port"].get<unsigned short>(),
				it.value()["addressFilter"].get<std::string>()
				) );
		}
	}

	if( !destinations.size() )
	{
		std::stringstream sstr;
		sstr << "no destinations specified in " << path;
		throw std::runtime_error( sstr.str() );
	}
}

void ctrlc( int sig )
{
	done = true;
}

void cleanup()
{
	if( server )
		lo_server_free( server );

	for( auto &it : destinations )
		delete( it );
	destinations.clear();
}

void __cdecl onExit()
{
	cleanup();
}

void __cdecl onTerminate()
{
	abort();
}

int main( int argc, char **argv )
{
	atexit( onExit );
	std::set_terminate( onTerminate );

	try
	{
		if( readPA( argc, argv ) )
		{
			readConfigFile( configFile );

			std::cout << "starting OSC server at UDP port " << oscPort << std::endl;
			server = lo_server_new( toString( oscPort ).c_str(), errorHandler );

			if( !server )
				throw std::runtime_error( "unable to start server" );

			lo_server_add_method( server, NULL, NULL, messageHandler, NULL );
			lo_server_add_bundle_handlers( server, bundleStartHandler, bundleEndHandler,
				NULL );

			signal( SIGINT, ctrlc );

			std::cout << "listening on local port " << oscPort << "..." << std::endl
				<< "hit Ctrl+C to quit" << std::endl;

			while( !done )
				lo_server_recv_noblock( server, 1 );
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