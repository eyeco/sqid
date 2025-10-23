/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "osc.h"

#include "../sceneGraph.h"

#include <fileIO/json.h>

#include <commonImGui.h>

#include <opencv2/imgproc.hpp>

#include <lo/lo.h>

#include <thread>
#include <string.h>

namespace sqid
{
	namespace OSC
	{
		DEFINE_OP_DESC( OSCOut, "oscOut", "/networking",
			"BE71EF33-3562-40E6-BB34-91C5FBDDADB8" );


		

		const char *outTypeToString( OSCOut::OutType type )
		{
			switch( type )
			{
			case OSCOut::OT_SAMPLEFRAME:
				return "sampleframe";
			case OSCOut::OT_NATIVE:
				return "native";
			case OSCOut::OT_NATIVE_CUSTOM:
				return "custom";
			}

			return "UNKNOWN";
		}

		OSCOut::OutType outTypeFromString( const char *s )
		{
			if( !s )
				return OSCOut::OT_COUNT;

			for( int i = 0; i < OSCOut::OT_COUNT; i++ )
				if( !_stricmp( s, outTypeToString( ( OSCOut::OutType ) i ) ) )
					return ( OSCOut::OutType ) i;

			return OSCOut::OT_COUNT;
		}

		OSCOut::OutType outTypeFromString( const std::string &s )
		{
			return outTypeFromString( s.c_str() );
		}

		const char *nativeTypeToString( OSCOut::NativeType nt )
		{
			switch( nt )
			{
			case OSCOut::NT_INT32:
				return "int32";
			case OSCOut::NT_FLOAT:
				return "float";
			//case OSCOut::NT_STRING:
			//	return "string";
			//case OSCOut::NT_BLOB:
			//	return "blob";
			case OSCOut::NT_INT64:
				return "int64";
			//case OSCOut::NT_TIMETAG:
			//	return "time";
			case OSCOut::NT_DOUBLE:
				return "double";
			//case OSCOut::NT_SYMBOL:
			//	return "symbol";
			case OSCOut::NT_CHAR:
				return "char";
			//case OSCOut::NT_MIDI:
			//	return "midi";
			//case OSCOut::NT_TRUE:
			//	return "true";
			//case OSCOut::NT_FALSE:
			//	return "false";
			//case OSCOut::NT_NIL:
			//	return "nil";
			//case OSCOut::NT_INF:
			//	return "inf";
			}

			return "UNKNOWN";
		}

		OSCOut::NativeType nativeTypeFromString( const char *s )
		{
			if( !s )
				return OSCOut::NT_COUNT;

			for( int i = 0; i < OSCOut::NT_COUNT; i++ )
				if( !_stricmp( s, nativeTypeToString( ( OSCOut::NativeType ) i ) ) )
					return ( OSCOut::NativeType ) i;

			return OSCOut::NT_COUNT;
		}

		OSCOut::NativeType nativeTypeFromString( const std::string &s )
		{
			return nativeTypeFromString( s.c_str() );
		}

		char loFromNativeType( OSCOut::NativeType nt )
		{
			switch( nt )
			{
			case OSCOut::NT_INT32: 
				return LO_INT32;
			case OSCOut::NT_FLOAT: 
				return LO_FLOAT;
			//case OSCOut::NT_STRING: 
			//	return LO_STRING;
			//case OSCOut::NT_BLOB: 
			//	return LO_BLOB;

			case OSCOut::NT_INT64: 
				return LO_INT64;
			//case OSCOut::NT_TIMETAG: 
			//	return LO_TIMETAG;
			case OSCOut::NT_DOUBLE: 
				return LO_DOUBLE;
			//case OSCOut::NT_SYMBOL: 
			//	return LO_SYMBOL;
			case OSCOut::NT_CHAR: 
				return LO_CHAR;
			//case OSCOut::NT_MIDI: 
			//	return LO_MIDI;
			//case OSCOut::NT_TRUE: 
			//	return LO_TRUE;
			//case OSCOut::NT_FALSE: 
			//	return LO_FALSE;
			//case OSCOut::NT_NIL: 
			//	return LO_NIL;
			//case OSCOut::NT_INF: 
			//	return LO_INFINITUM;
			}

			throw std::runtime_error( "unknown lo type" );
		}



		OSCOut::OSCOut( const std::string &ip, unsigned short port, const std::string &oscPath ) :
			Op(),
			_sending( false ),
			_proto( P_UDP ),
			_ip( ip ),
			_port( port ),
			_path( oscPath ),
			_outType( OT_SAMPLEFRAME ),
			_nativeType( NT_FLOAT ),
			_inputBufferIP( 32 ),
			_inputBufferPort( 16 ),
			_inputBufferPath( 256 ),
			_address( nullptr )
		{
			updateBuffers();
			open();
		}

		OSCOut::~OSCOut()
		{
			close();
		}

		bool OSCOut::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				if( _sending )
				{
					if( _address )
					{
						if( sf->size() > PACKAGE_MAX_SAMPLES && _proto == P_UDP )
							std::cout << "<error> sampleframe with " << sf->size() << " samples is most certainly too tall to be sent via OSC -- package will not be sent" << std::endl;
						else
						{
							if( sf->size() > 100 && _proto == P_UDP )
							{
								//NOTE: according to RFC 791 [1], only 576 bytes are guaranteed (subtract osc overhead and custom sampleframe header to get to something 
								// like ~120 samples). if you go beyond that, make sure
								// - the receiver can handle the data (check input package sizes and watch out for input buffer overflow)
								// - output buffer does not overflow
								// - the sender can handle packages of this size without having to fragment
								// [1] https://tools.ietf.org/html/rfc791 
								std::cout << "<warning> sampleframe size is borderline for sending OSC packages with " << sf->size() << " samples." << std::endl;
							}

							bool ret = true;

							lo_message message = lo_message_new();

							try
							{
								std::string path( _path );

								if( _outType == OT_SAMPLEFRAME )
								{
									lo_blob blob = nullptr;

									lo_message_add_int32( message, (int32_t) sf->width() );
									lo_message_add_int32( message, (int32_t) sf->height() );
									lo_message_add_int32( message, (int32_t) sf->depth() );
									lo_message_add_int32( message, (int32_t) sf->timeStamp() );

									if( sf->size() )
										blob = lo_blob_new( sf->size() * sizeof( float ), &sf->values()[0] );
									else
									{
										//cannot send empty blob (but prefer to send blob since receivers mostly validate by number of OSC arguments)
										float dummy = 0;
										blob = lo_blob_new( sizeof( float ), &dummy );
									}
									lo_message_add_blob( message, blob );

									if( blob )
										lo_blob_free( blob );
								}
								else if( _outType == OT_NATIVE )
								{
									const float *ptr = sf->values();
									for( int i = 0; i < sf->size(); i++, ptr++ )
									{
										//TODO: doublecheck if sf is too big for osc package, and/or handle error coming from lo_message_add*
										switch( _nativeType )
										{
										case NT_INT32:
											lo_message_add_int32( message, (int32_t) *ptr );
											break;
										case NT_FLOAT:
											lo_message_add_float( message, (float) *ptr );
											break;
										case NT_INT64:
											lo_message_add_int64( message, (int64_t) *ptr );
											break;
										case NT_DOUBLE:
											lo_message_add_double( message, (double) *ptr );
											break;
										case NT_CHAR:
											lo_message_add_char( message, (char) *ptr );
											break;
										default:
											throw std::runtime_error( "type not implemented" );
										}
									}
								}
								else
								{
									throw std::runtime_error( "OT_NATIVE_CUSTOM not implemented" );
								}

								if( lo_send_message( _address, path.c_str(), message ) < 0 )
								{
									std::cerr << "<error> failed sending OSC message (UDP)" << std::endl;
									close();
								}
							}
							catch( std::exception &e )
							{
								std::cerr << "<error> unable to send " << _path << " (" << _ip << ":" << _port << "): " << e.what() << std::endl;
								ret = false;
							}

							if( message )
								lo_message_free( message );
						}
					}
					else
					{
						std::cerr << "<error> cannot send, socket not set up" << std::endl;
						setEnabled( false );
					}
				}

				drawFrame( sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool OSCOut::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _sending ? "stop" : "start" ) )
				_sending = !_sending;

			{
				ScopedImGuiDisable disable( _sending );

				int p = (int) _proto;
				for( int i = 0; i < FF_COUNT; i++ )
					ImGui::RadioButton( protocolToString( (Protocol) i ), &p, i );
				if( _proto != (Protocol) p )
				{
					_proto = (Protocol) p;

					open();
				}

				if( ImGui::InputText( "ip", &_inputBufferIP[0], _inputBufferIP.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					bool valid = true;

					std::string str = trim( &_inputBufferIP[0] );
					std::vector<std::string> subs = split( str, '.' );
					if( subs.size() != 4 )
						valid = false;
					else for( auto it = subs.begin(); it != subs.end(); ++it )
						for( auto it2 = it->begin(); it2 != it->end(); ++it2 )
							if( *it2 < '0' || *it2 > '9' )
								valid = false;

					if( valid )
					{
						_ip = str;

						open();
					}
					else
						strcpy( &_inputBufferIP[0], _ip.c_str() );

					updateBuffers();
				}

				if( ImGui::InputText( "port", &_inputBufferPort[0], _inputBufferPort.size(), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					_port = atoi( &_inputBufferPort[0] );

					open();

					updateBuffers();
				}

				if( ImGui::InputText( "path", &_inputBufferPath[0], _inputBufferPath.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					std::string str = trim( &_inputBufferPath[0] );
					if( str.size() )
					{
						if( str[0] != '/' )
							_path = "/" + str;
						else
							_path = str;
					}
					else
						_path = "/";

					updateBuffers();
				}

				bool native = ( _outType == OT_NATIVE || _outType == OT_NATIVE_CUSTOM );
				bool custom = ( _outType == OT_NATIVE_CUSTOM );
				ImGui::Checkbox( "OSC native types", &native );
				ImGui::SameLine();
				ImGui::BeginDisabled();
				ImGui::Checkbox( "OSC custom", &custom );
				ImGui::EndDisabled();

				if( native )
				{
					if( custom )
					{
						//TODO: implement
						//_outType = OT_NATIVE_CUSTOM;
					}
					else
					{
						_outType = OT_NATIVE;

						int e = (int)_nativeType;
						for( int i = 0; i < (int)OSCOut::NT_COUNT; i++ )
							if( ImGui::RadioButton( nativeTypeToString( (OSCOut::NativeType)i ), i == e ) )
								_nativeType = (OSCOut::NativeType)i;
					}
				}
				else
					_outType = OT_SAMPLEFRAME;
			}

			return true;
		}
#endif

		void OSCOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

		bool OSCOut::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "sending", _sending );
			std::string str;
			if( load<std::string>( j, "proto", str ) )
				_proto = protocolFromString( str );
			load<std::string>( j, "ip", _ip );
			load<uint16_t>( j, "port", _port );
			load<std::string>( j, "oscPath", _path );
			if( load<std::string>( j, "outType", str ) )
				_outType = outTypeFromString( str );
			if( load<std::string>( j, "nativeType", str ) )
				_nativeType = nativeTypeFromString( str );

			open();
			updateBuffers();

			return ret;
		}

		bool OSCOut::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "sending", _sending );
			save( j, "proto", protocolToString( _proto ) );
			save( j, "ip", _ip );
			save( j, "port", _port );
			save( j, "oscPath", _path );
			save( j, "outType", outTypeToString( _outType ) );
			save( j, "nativeType", nativeTypeToString( _nativeType ) );

			return ret;
		}

		void OSCOut::updateBuffers()
		{
			strncpy( &_inputBufferIP[0], _ip.c_str(), _ip.size() + 1 );
			snprintf( &_inputBufferPort[0], _inputBufferPort.size() - 1, "%d", _port );
			strncpy( &_inputBufferPath[0], _path.c_str(), _path.size() + 1 );
		}

		bool OSCOut::open()
		{
			close();

			int proto = LO_DEFAULT;

			switch( _proto )
			{
			case P_UDP:
				proto = LO_UDP;
				break;
			case P_TCP:
				proto = LO_TCP;
				break;
			default:
				std::cerr << "<error> unknown protocol" << std::endl;
				return false;
			}

			_address = lo_address_new_with_proto( proto, _ip.c_str(), toString( _port ).c_str() );
			if( !_address )
			{
				std::cerr << "<error> creating address " << _ip << ":" << _port << " failed (" << protocolToString( _proto ) << ")" << std::endl;
				return false;
			}
			
			std::cout << "created osc socket " << _ip << ":" << _port << "(" << protocolToString( _proto ) << ")" << std::endl;
			if( _proto == P_TCP )
			{
				std::cout << "sending TCP test package, trying to connect..." << std::endl;
				if( lo_send( _address, "/", "" ) < 0 )	//send dummy message to see if there is a server at all, otherwise sending in process() would block for quite a while
				{
					std::cerr << "<error> sending TCP test package failed, closing again." << std::endl;
					close();
					return false;
				}
			}

			return true;
		}

		void OSCOut::close()
		{
			if( _address )
			{
				lo_address_free( _address );
				_address = nullptr;
			}
		}
	}
}