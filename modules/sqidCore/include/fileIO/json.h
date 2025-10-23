/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <config.h>

#include <nlohmann/json.hpp>

#include <guid.h>
#include <sampleFrame.h>

#include <iostream>

namespace sqid
{
	template<typename T>
	bool SQID_API_CALL load( const nlohmann::json &j, const char *name, T &t )
	{
		try
		{
#ifdef _DEBUG
			//NOTE: unfortunately, nlohmann::json does not throw in Debug mode, instead it raises an assert,
			// so we have to check for ourselves if we don't want our code to crash here.
			if( !j.count( name ) )
				throw std::runtime_error( "field not found" );
#endif

			t = j[name].get<T>();
		}
		catch( std::exception &e )
		{
			std::cerr << "<error> failed reading field \"" << name << "\" from JSON: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	template<> SQID_API bool SQID_API_CALL  load<std::string>( const nlohmann::json &j, const char *name, std::string &str );
	template<> SQID_API bool SQID_API_CALL  load<glm::vec2>( const nlohmann::json &j, const char *name, glm::vec2 &v );
	template<> SQID_API bool SQID_API_CALL  load<glm::vec3>( const nlohmann::json &j, const char *name, glm::vec3 &v );
	template<> SQID_API bool SQID_API_CALL  load<glm::vec4>( const nlohmann::json &j, const char *name, glm::vec4 &v );
	template<> SQID_API bool SQID_API_CALL  load<glm::mat4>( const nlohmann::json &j, const char *name, glm::mat4 &m );
	template<> SQID_API bool SQID_API_CALL  load<GUID>( const nlohmann::json &j, const char *name, GUID &guid );
	template<> SQID_API bool SQID_API_CALL  load<SampleFrame*>( const nlohmann::json &j, const char *name, SampleFrame* &sf );

	template<typename T>
	bool SQID_API_CALL save( nlohmann::json &j, const char *name, const T &value )
	{
		try
		{
			j[name] = value;
		}
		catch( std::exception )
		{
			std::cerr << "<error> failed writing field \"" << name << "\" to JSON" << std::endl;
			return false;
		}
		return true;
	}

	template<> SQID_API bool SQID_API_CALL  save<glm::vec2>( nlohmann::json &j, const char *name, const glm::vec2 &v );
	template<> SQID_API bool SQID_API_CALL  save<glm::vec3>( nlohmann::json &j, const char *name, const glm::vec3 &v );
	template<> SQID_API bool SQID_API_CALL  save<glm::vec4>( nlohmann::json &j, const char *name, const glm::vec4 &v );
	template<> SQID_API bool SQID_API_CALL  save<glm::mat4>( nlohmann::json &j, const char *name, const glm::mat4 &m );
	template<> SQID_API bool SQID_API_CALL  save<GUID>( nlohmann::json &j, const char *name, const GUID &guid );
	template<> SQID_API bool SQID_API_CALL  save<SampleFrame>( nlohmann::json &j, const char *name, const SampleFrame &sf );
}