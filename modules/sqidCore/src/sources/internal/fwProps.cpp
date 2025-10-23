/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "fwProps.h"

#include <common.h>
#include <commonImGui.h>

#include <iostream>

#ifdef __SUPPORT_GUI
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#endif

namespace sqid
{
	namespace Internal
	{
		FWProps::FWProps( IAbstractWriter *writer ) :
			autoSet( false ),
			writer( writer )
		{}

		FWProps::~FWProps()
		{
			clear();
			writer = nullptr;
		}

#ifdef __SUPPORT_GUI
		void FWProps::drawUI()
		{
			ImGui::BeginGroup();
			ImGui::Text( "FW props" );

			bool empty = ( props.size() == 0 );

			if( ImGui::Button( "init" ) )
				requestDesc();
			ImGui::SameLine();

			{
				ScopedImGuiDisable disable( empty );

				if( ImGui::Button( "status" ) )
					requestStatus();
			}

			for( auto &it : props )
				it.second.drawUI();

			ImGui::Checkbox( "auto set", &autoSet );

			if( autoSet )
				for( auto &it : props )
					if( it.second.ptr->isDirty() )
						set( it.second.name );

			{
				ScopedImGuiDisable disable( empty || autoSet );

				if( ImGui::Button( "set" ) )
					for( auto &it : props )
						set( it.second.name );

				if( autoSet && !empty )
					disable.enable();

				ImGui::SameLine();
				if( ImGui::Button( "reset" ) )
					reset();
			}

			ImGui::EndGroup();
		}
#endif

		bool FWProps::onDesc( const unsigned char *data, size_t size )
		{
			clear();

			std::string desc( reinterpret_cast<const char*>( data ), size );

			auto entries = split( desc, ';' );
			for( auto entry : entries )
			{
				std::string s( entry );
				trim( s );

				if( s.size() )
				{
					auto subs = split( s, ':' );
					if( subs.size() != 3 )
					{
						std::cerr << "<warning> invalid desc entry in FW properties: \"" << entry << "\"" << std::endl;
						continue;
					}

					if( subs[0].size() != 1 )
					{
						std::cerr << "<warning> invalid desc entry in FW properties: \"" << entry << "\"" << std::endl;
						continue;
					}

					Prop p;
					p.name = subs[1];

					switch( subs[0][0] )
					{
					case 'B':
						p.type = DT_BYTE;
						p.ptr = createPropContainer<uint8_t>( subs[2] );
						break;
					case 'S':
						p.type = DT_SHORT;
						p.ptr = createPropContainer<uint16_t>( subs[2] );
						break;
					case 'L':
						p.type = DT_LONG;
						p.ptr = createPropContainer<uint32_t>( subs[2] );
						break;
					case 'F':
						p.type = DT_FLOAT;
						p.ptr = createPropContainer<float>( subs[2] );
						break;
					default:
						std::cerr << "<warning> unknown FW property type '" << subs[0][0] << "'" << std::endl;
					}

					if( !p.ptr )
					{
						std::cerr << "<error> failed to create FW prop entry from description \"" << subs[2] << "\"" << std::endl;
						continue;
					}

					props.insert( std::make_pair( p.name, p ) );
				}
			}

			return true;
		}

		bool FWProps::onStatus( const unsigned char *data, size_t size )
		{
			std::string status( reinterpret_cast<const char*>( data ), size );

			auto entries = split( status, ';' );
			for( auto entry : entries )
			{
				std::string s( entry );
				trim( s );

				if( s.size() )
				{
					auto subs = split( s, ':' );
					if( subs.size() != 2 )
					{
						std::cerr << "<warning> invalid status entry in FW properties: \"" << entry << "\"" << std::endl;
						continue;
					}

					if( !subs[0].size() )
					{
						std::cerr << "<warning> invalid status entry in FW properties: \"" << entry << "\"" << std::endl;
						continue;
					}

					auto it = props.find( subs[0] );
					if( it == props.end() )
					{
						std::cerr << "<error> property with name \"" << subs[0] << "\" not found" << std::endl;
						continue;
					}

					if( !it->second.ptr->parse( subs[1] ) )
					{
						std::cerr << "<error> failed to parse property with name \"" << subs[0] << "\" from string \"" << subs[1] << "\"" << std::endl;
						continue;
					}

					it->second.ptr->clearDirtyFlag();
				}
			}

			return true;
		}

		bool FWProps::onAck( const unsigned char *data, size_t size )
		{
			//std::string ack( reinterpret_cast<const char*>( data ), size );
			return true;
		}

		bool FWProps::requestDesc()
		{
			static std::string command( "desc;" );

			if( !writer )
				return false;
			return writer->write( reinterpret_cast<const unsigned char*>( command.c_str() ), command.size() );
		}

		bool FWProps::requestStatus()
		{
			static std::string command( "status;" );

			if( !writer )
				return false;
			return writer->write( reinterpret_cast<const unsigned char*>( command.c_str() ), command.size() );
		}

		bool FWProps::set( const std::string &name )
		{
			if( !writer )
				return false;

			auto it = props.find( name );
			if( it == props.end() )
				return false;

			std::string value = it->second.ptr->toString();

			std::stringstream sstr;
			sstr << "set " << it->second.name << ":" << value << ";";

			std::string s( sstr.str() );
			if( writer->write( reinterpret_cast<unsigned const char*>( s.c_str() ), s.size() ) )
			{
				it->second.ptr->clearDirtyFlag();
				return true;
			}

			return false;
		}

		bool FWProps::reset()
		{
			static std::string command( "reset;" );

			if( !writer )
				return false;
			return writer->write( reinterpret_cast<const unsigned char*>( command.c_str() ), command.size() );
		}

#ifdef __SUPPORT_GUI
		template<> void FWProps::PropContainer<float>::drawUI( const std::string &name )
		{
			float v = value;
			bool highlight = dirty;

			{
				ScopedImGuiStyleColor style;
				if( highlight )
					style.set( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );

				if( ImGui::SliderFloat( name.c_str(), &v, minValue, maxValue ) )
				{
					dirty = true;
					value = v;
				}
			}
		}
#endif

		template<> bool FWProps::PropContainer<float>::parse( const std::string &s )
		{
			float v = 0;
			if( sscanf( s.c_str(), "%f", &v ) != 1 )
				return false;

			value = (float) v;

			return true;
		}

		template<> std::string FWProps::PropContainer<uint8_t>::toString()
		{
			std::stringstream sstr;
			sstr << (int) value;

			return sstr.str();
		}

		void FWProps::clear()
		{
			for( auto &it : props )
				safeDelete( it.second.ptr );
			props.clear();
		}





		template<> bool FWProps::parseDesc<float>( const std::string &desc, float &value, float &defValue, float &minValue, float &maxValue )
		{
			if( sscanf( desc.c_str(), "%f<%f>[%f %f]", &value, &defValue, &minValue, &maxValue ) != 4 )
				return false;

			return true;
		}
	}
}