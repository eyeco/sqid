/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>

#include <conio.h>
#include <Windows.h>

#include <lo/lo.h>


std::string defaultIP = "127.0.0.1";
const unsigned short defaultPort = 6667;

std::string currentIP;
unsigned short currentPort = 0;

lo_address target = nullptr;
lo_message message = nullptr;

template<typename T>
static std::string toString( const T &t )
{
	std::stringstream sstr;
	sstr << t;

	return sstr.str();
}

void reset()
{
	currentIP = defaultIP;
	currentPort = defaultPort;
	if( target )
		lo_address_free( target );
	target = lo_address_new( currentIP.c_str(), toString( currentPort ).c_str() );
}

void showHelp()
{
	std::cout 
		<< "entry starting with a slash (e.g., \"/my/target/osc/address\") will be interpreted as OSC messages and sent along stated OSC aruments" << std::endl
		<< "OSC arguments need to be prefxied with a backslash-escaped type specifier, e.g. \\i123 for integer 123. if type prefix is missing, argument will be sent as string." << std::endl
		<< "  available types are: " << std::endl
		<< "    i            32-bit signed integer" << std::endl
		<< "    f            32-bit IEEE 754 float" << std::endl
		<< "    s            null-terminated C-string" << std::endl
		<< "    b            OSC binary blob (e.g. \\b123456 to send ASCII \"123456\" as a 6-byte blob)" << std::endl
		<< "    h            64-bit signed integer" << std::endl
		<< "    t            timetag (e.g. \"1234:5678\")" << std::endl
		<< "    d            64-bit IEEE-754 double" << std::endl
		<< "    S            OSC symbol, sent as null-terminated C-string" << std::endl
		<< "    c            8-bit char" << std::endl
		<< "    m            4-byte MIDI packet, needs to be specified as HEX string, e.g. \"DEADBEEF\"" << std::endl
		<< "    T            symbol representing TRUE, no values required, just put \"\\T\"" << std::endl
		<< "    F            symbol representing FALSE, no values required, just put \"\\T\"" << std::endl
		<< "    N            symbol representing Nil, no values required, just put \"\\T\"" << std::endl
		<< "    I            symbol representing Infinitum, no values required, just put \"\\T\"" << std::endl
		<< "  available commands: " << std::endl
		<< "    quit, exit   quit program" << std::endl
		<< "    h, help      show this help text" << std::endl
		<< "    a <address>  change target address (currently restricted to IP)" << std::endl
		<< "    p <port>     change target port (currently restricted to unicast UDP)" << std::endl
		<< "    reset        reset to default target address and port" << std::endl;
}

void cleanup()
{
	if( target )
		lo_address_free( target );
	if( message )
		lo_message_free( message );
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

const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 50;
const char* const DELIMITER = " ";

int main( int argc, char **argv )
{
	atexit( onExit );
	std::set_terminate( onTerminate );
	std::set_unexpected( onUnexpected );

	try
	{
		reset();

		bool quit = false;
		while( !quit )
		{
			std::cout << lo_address_get_hostname( target ) << ":" << lo_address_get_port( target ) << ">";

			char buf[MAX_CHARS_PER_LINE];

			std::cin.getline( buf, MAX_CHARS_PER_LINE);

			try
			{
				int n = 0;
				std::vector<std::string> token_1[MAX_TOKENS_PER_LINE] = {};
				const char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0

				token[0] = strtok( buf, DELIMITER );
				if( token[0] )
				{
					for( n = 1; n < MAX_TOKENS_PER_LINE; n++ )
					{
						token[n] = strtok( 0, DELIMITER );

						if( !token[n] )
							break;
					}
				}

				if( n )
				{
					if( !_stricmp( token[0], "exit" ) || !_stricmp( token[0], "quit" ))
						quit = true;
					else if( !_stricmp( token[0], "h" ) || !_stricmp( token[0], "help" ) )
						showHelp();
					else if( !_stricmp( token[0], "a" ) )
					{
						if( n != 2 )
							throw std::runtime_error( "syntax error" );

						currentIP = token[1];
						lo_address_free( target );
						target = lo_address_new( currentIP.c_str(), toString( currentPort ).c_str() );
					}
					else if( !_stricmp( token[0], "p" ) )
					{
						if( n != 2 )
							throw std::runtime_error( "syntax error" );

						currentPort = atoi( token[1] );
						lo_address_free( target );
						target = lo_address_new( currentIP.c_str(), toString( currentPort ).c_str() );
					}
					else if( !_stricmp( token[0], "reset" ) )
					{
						reset();
					}
					else if( token[0][0] == '/' )
					{
						message = lo_message_new();

						for( int i = 1; i < n; i++ )
						{

							if( token[i][0] == '\\' )
							{
								if( strlen( token[i] ) < 3 )
								{
									lo_message_free( message );
									message = nullptr;

									throw std::runtime_error( "syntax error" );
								}

								switch( token[i][1] )
								{
								/* basic OSC types */
								/** 32 bit signed integer. */
								case LO_INT32:
									lo_message_add_int32( message, (int32_t)atoi( token[i] + 2 ) );
									break;
								/** 32 bit IEEE-754 float. */
								case LO_FLOAT:
									lo_message_add_float( message, (float) atof( token[i] + 2 ) );
									break;
								/** Standard C, NULL terminated string. */
								case LO_STRING:
									lo_message_add_string( message, token[i] + 2 );
									break;
								/** OSC binary blob type. Accessed using the case LO_blob_*() functions. */
								case LO_BLOB:
								{
									lo_blob b = lo_blob_new( strlen( token[i] + 2 ), token[i] + 2 );
									lo_message_add_blob( message, b );
									lo_blob_free( b );
									break;
								}

								/* extended OSC types */
								/** 64 bit signed integer. */
								case LO_INT64:
									lo_message_add_int64( message, (int64_t) _atoi64( token[i] + 2 ) );
									break;
								/** OSC TimeTag type, represented by the case LO_timetag structure. */
								case LO_TIMETAG:
								{
									int sec = 0;
									int frac = 0;
									if( sscanf( token[i] + 2, "%d:%d", sec, frac ) != 2 )
									{
										lo_message_free( message );
										message = nullptr;

										throw std::runtime_error( "syntax error: timetag formating must be \"sec:frac\", e.g. \"1234:567\"" );
									}

									lo_timetag tt = { sec, frac };
									lo_message_add_timetag( message, tt );
									break;
								}

								/** 64 bit IEEE-754 double. */
								case LO_DOUBLE:
									lo_message_add_double( message, (double) atof( token[i] + 2 ) );
									break;
								/** Standard C, NULL terminated, string. Used in systems which
									* distinguish strings and symbols. */
								case LO_SYMBOL:
									lo_message_add_symbol( message, token[i] + 2 );
									break;
								/** Standard C, 8 bit, char variable. */
								case LO_CHAR:
									lo_message_add_char( message, *( token[i] + 2 ) );
									break;
								/** A 4 byte MIDI packet. */
								case LO_MIDI:
								{
									unsigned int midi;

									if( sscanf( token[i] + 2, "%08x", &midi ) != 1 )
									{
										lo_message_free( message );
										message = nullptr;

										throw std::runtime_error( "syntax error: MIDI message must be given as hex string representing 4 bytes, e.g. \"BAADF00D\"" );
									}

									uint8_t packet[4];
									packet[0] = ( midi >> 24 ) & 0xff;
									packet[1] = ( midi >> 16 ) & 0xff;
									packet[2] = ( midi >> 8 ) & 0xff;
									packet[3] = midi & 0xff;

									lo_message_add_midi( message, packet );
									break;
								}
								/** Sybol representing the value True. */
								case LO_TRUE:
									if( strlen( token[i] + 2 ) )
										std::cerr << "<warning> value of TRUE field will be ignored" << std::endl;
									lo_message_add_true( message );
									break;
								/** Sybol representing the value False. */
								case LO_FALSE:
									if( strlen( token[i] + 2 ) )
										std::cerr << "<warning> value of FALSE field will be ignored" << std::endl;
									lo_message_add_false( message );
									break;
								/** Sybol representing the value Nil. */
								case LO_NIL:
									if( strlen( token[i] + 2 ) )
										std::cerr << "<warning> value of NIL field will be ignored" << std::endl;
									lo_message_add_nil( message );
									break;
								/** Sybol representing the value Infinitum. */
								case LO_INFINITUM:
									if( strlen( token[i] + 2 ) )
										std::cerr << "<warning> value of INF field will be ignored" << std::endl;
									lo_message_add_infinitum( message );
									break;
								default:
								{
									std::stringstream sstr;
									sstr << "unknown argument type '" << token[i][1] << "'";
									throw std::runtime_error( sstr.str() );
								}
								}
							}
							else
								lo_message_add_string( message, token[i] );
						}

						lo_send_message( target, token[0], message );

						lo_message_free( message );
						message = nullptr;
					}
					else 
						std::cout << "<warning> OSC address must start with '/', otherwise ignored. enter \"h\" for help" << std::endl;
				}
			}
			catch( std::exception &e )
			{
				std::cerr << "caught exception parsing line: " << e.what() << std::endl;
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