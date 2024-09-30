/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


//#include <osc/OscReceivedElements.h>
//#include <osc/OscPacketListener.h>
//#include <ip/UdpSocket.h>

#include <lo/lo.h>

#include <fstream>
#include <iostream>
#include <exception>

#include <conio.h>
#include <signal.h>

#include <Windows.h>

#include <cxxopts.hpp>

unsigned short port = 6667;
std::string url;

bool done = false;
bool bundled = false;
lo_timetag tt_now;
lo_timetag tt_bundle;

lo_server server = nullptr;

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

	std::cout << "<" << tt.sec << "." << tt.frac << "> src: " << srcHost << ":" << srcPort << " [" << srcProtoStr << "]; local port: " << port << std::endl
		<< "\t" << path << std::endl;

	char tempStr[128];

	for( int i = 0; i < argc; i++ )
	{
		//printf( " " );
		//lo_arg_pp( (lo_type) types[i], argv[i] );

		std::cout << "\t\t";

		switch( types[i] )
		{
		/* basic OSC types */
		/** 32 bit signed integer. */
		case LO_INT32:
			std::cout << "[int32] " << argv[i]->i32;
			break;
		/** 32 bit IEEE-754 float. */
		case LO_FLOAT:
			std::cout << "[float] " << argv[i]->f32;
			break;
		/** Standard C, NULL terminated string. */
		case LO_STRING:
			std::cout << "[cstr] " << &( argv[i]->s );
			break;
		/** OSC binary blob type. Accessed using the lo_blob_*() functions. */
		case LO_BLOB:
			std::cout << "[blob] " << argv[i]->blob.size << " bytes";
			break;

		/* extended OSC types */
		/** 64 bit signed integer. */
		case LO_INT64:
			std::cout << "[int64] " << argv[i]->i64;
			break;
		/** OSC TimeTag type, represented by the lo_timetag structure. */
		case LO_TIMETAG:
			std::cout << "[timetag] " << argv[i]->t.sec << ":" << argv[i]->t.frac;
			break;
		/** 64 bit IEEE-754 double. */
		case LO_DOUBLE:
			std::cout << "[double] " << argv[i]->f64;
			break;
		/** Standard C, NULL terminated, string. Used in systems which
			* distinguish strings and symbols. */
		case LO_SYMBOL:
			std::cout << "[symbol] " << &( argv[i]->S );
			break;
		/** Standard C, 8 bit, char variable. */
		case LO_CHAR:
			sprintf( tempStr, "0x%02x", argv[i]->c );
			std::cout << "[char] '" << argv[i]->c << "' " << tempStr;
			break;
		/** A 4 byte MIDI packet. */
		case LO_MIDI:
			sprintf( tempStr, "%02x%02x%02x%02x HEX", argv[i]->m[0], argv[i]->m[1], argv[i]->m[2], argv[i]->m[3] );
			std::cout << "[MIDI] " << tempStr;
			break;
		/** Sybol representing the value True. */
		case LO_TRUE:
			std::cout << "[TRUE]";
			break;
		/** Sybol representing the value False. */
		case LO_FALSE:
			std::cout << "[FALSE]";
			break;
		/** Sybol representing the value Nil. */
		case LO_NIL:
			std::cout << "[NIL]";
			break;
		/** Sybol representing the value Infinitum. */
		case LO_INFINITUM:
			std::cout << "[INF]";
			break;
		}

		std::cout << std::endl;
	}

	return 0;
}






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
		( "p,port", "OSC target port", cxxopts::value<unsigned short>()->default_value( toString( port ) ) )
		( "u,url", "specifies the server parameters using a liblo URL.\n"
					"          e.g. UDP        \"osc.udp://:9000\"\n"
					"               Multicast  \"osc.udp://224.0.1.9:9000\"\n"
					"               TCP        \"osc.tcp://:9000\"", cxxopts::value<std::string>() )
		;

	//positional options:
	options.parse_positional( { "p" } );

	try
	{
		cxxopts::ParseResult result = options.parse( argc, argv );

		//generic options
		if( result.count( "help" ) )
		{
			std::cout << options.help( { "", "OSC" } ) << std::endl;
			return false;
		}

		port = result["port"].as<unsigned short>();
		if( result.count( "url" ) )
			url = result["url"].as<std::string>();
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

void ctrlc( int sig )
{
	done = true;
}

void cleanup()
{
	if( server )
		lo_server_free( server );
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
			if( url.size() )
			{
				std::cout << "starting OSC server from URL " << url << std::endl;
				server = lo_server_new_from_url( url.c_str(), errorHandler );
			}
			else
			{
				std::cout << "starting OSC server at UDP port " << port << std::endl;
				server = lo_server_new( toString( port ).c_str(), errorHandler );
			}

			if( !server )
				throw std::runtime_error( "unable to start server" );

			lo_server_add_method( server, NULL, NULL, messageHandler, NULL );
			lo_server_add_bundle_handlers( server, bundleStartHandler, bundleEndHandler,
				NULL );

			signal( SIGINT, ctrlc );

			while( !done )
			{
				lo_server_recv_noblock( server, 1 );
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