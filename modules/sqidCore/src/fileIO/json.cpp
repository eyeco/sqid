/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <fileIO/json.h>

namespace sqid
{
	template<>
	SQID_API bool SQID_API_CALL load<std::string>( const nlohmann::json &j, const char *name, std::string &str )
	{
		try
		{
			if(!j.count(name))
				throw std::runtime_error("key not found in JSON");
			str = j[name].get<std::string>();
		}
		catch( std::exception &e )
		{
			std::cerr << "<error> failed reading field \"" << name << "\" from JSON: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	template<>
	SQID_API bool SQID_API_CALL load<glm::vec2>( const nlohmann::json &j, const char *name, glm::vec2 &v )
	{
		std::string str;
		if( !load<std::string>( j, name, str ) )
			return false;

		std::stringstream sstr;
		sstr << str;

		glm::vec2 temp;
		float *ptr = glm::value_ptr( temp );

		int cntr = 0;
		std::string sub;
		while( std::getline( sstr, sub, ',' ) )
		{
			if( cntr > 2 )
				throw std::runtime_error( "error reading vec2 from JSON" );
			ptr[cntr++] = atof( sub.c_str() );
		}

		v = temp;
		return true;
	}

	template<>
	SQID_API bool SQID_API_CALL load<glm::vec3>( const nlohmann::json &j, const char *name, glm::vec3 &v )
	{
		std::string str;
		if( !load<std::string>( j, name, str ) )
			return false;

		std::stringstream sstr;
		sstr << str;

		glm::vec3 temp;
		float *ptr = glm::value_ptr( temp );

		int cntr = 0;
		std::string sub;
		while( std::getline( sstr, sub, ',' ) )
		{
			if( cntr > 3 )
				throw std::runtime_error( "error reading vec3 from JSON" );
			ptr[cntr++] = atof( sub.c_str() );
		}

		v = temp;
		return true;
	}

	template<>
	SQID_API bool SQID_API_CALL load<glm::vec4>( const nlohmann::json &j, const char *name, glm::vec4 &v )
	{
		std::string str;
		if( !load<std::string>( j, name, str ) )
			return false;

		std::stringstream sstr;
		sstr << str;

		glm::vec4 temp;
		float *ptr = glm::value_ptr( temp );

		int cntr = 0;
		std::string sub;
		while( std::getline( sstr, sub, ',' ) )
		{
			if( cntr > 4 )
				throw std::runtime_error( "error reading vec4 from JSON" );
			ptr[cntr++] = atof( sub.c_str() );
		}

		v = temp;
		return true;
	}

	template<>
	SQID_API bool SQID_API_CALL load<glm::mat4>( const nlohmann::json &j, const char *name, glm::mat4 &m )
	{
		std::string str;
		if( !load<std::string>( j, name, str ) )
			return false;

		std::stringstream sstr;
		sstr << str;

		glm::mat4 temp;
		float *ptr = glm::value_ptr( temp );

		int cntr = 0;
		std::string sub;
		while( std::getline( sstr, sub, ',' ) )
		{
			if( cntr > 16 )
				throw std::runtime_error( "error reading mat4 from JSON" );
			ptr[cntr++] = atof( sub.c_str() );
		}

		m = temp;
		return true;
	}

	template<>
	SQID_API bool SQID_API_CALL load<GUID>( const nlohmann::json &j, const char *name, GUID &guid )
	{
		try
		{
			guid = guidFromString( j[name].get<std::string>() );
		}
		catch( std::exception &e )
		{
			std::cerr << "<error> failed reading field \"" << name << "\" from JSON: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	template<>
	SQID_API bool SQID_API_CALL load<SampleFrame*>( const nlohmann::json &j, const char *name, SampleFrame* &sf )
	{
		try
		{
			sf = SampleFrame::createFromJSON( j[name] );
		}
		catch( std::exception &e )
		{
			std::cerr << "<error> failed reading field \"" << name << "\" from JSON: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	template<>
	SQID_API bool SQID_API_CALL save<glm::vec2>( nlohmann::json &j, const char *name, const glm::vec2 &v )
	{
		std::stringstream sstr;
		sstr << v.x << "," << v.y << ",";
		return save( j, name, sstr.str() );
	}

	template<>
	SQID_API bool SQID_API_CALL save<glm::vec3>( nlohmann::json &j, const char *name, const glm::vec3 &v )
	{
		std::stringstream sstr;
		sstr << v.x << "," << v.y << "," << v.z << ",";
		return save( j, name, sstr.str() );
	}

	template<>
	SQID_API bool SQID_API_CALL save<glm::vec4>( nlohmann::json &j, const char *name, const glm::vec4 &v )
	{
		std::stringstream sstr;
		sstr << v.x << "," << v.y << "," << v.z << "," << v.w << ",";
		return save( j, name, sstr.str() );
	}

	template<>
	SQID_API bool SQID_API_CALL save<glm::mat4>( nlohmann::json &j, const char *name, const glm::mat4 &m )
	{
		const float *v = glm::value_ptr( m );
		std::stringstream sstr;
		for( int i = 0; i < 16; i++ )
			sstr << v[i] << ",";
		return save( j, name, sstr.str() );
	}

	template<>
	SQID_API bool SQID_API_CALL save<GUID>( nlohmann::json &j, const char *name, const GUID &guid )
	{
		return save( j, name, guidToString( guid ) );
	}

	template<>
	SQID_API bool SQID_API_CALL save<SampleFrame>( nlohmann::json &j, const char *name, const SampleFrame &sf )
	{
		try
		{
			SampleFrame::saveToJSON( j[name], &sf );
		}
		catch( std::exception &e )
		{
			std::cerr << "<error> failed writing field \"" << name << "\" to JSON: \"" << e.what() << "\"" << std::endl;
			return false;
		}
		return true;
	}
}