/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <fileIO/iniFile.h>

#include <common.h>

#include <iostream>
#include <filesystem>

namespace sqid
{
	void IniFile::Section::write( std::fstream &file )
	{
		file << "[" << _name << "]" << std::endl;

		for( auto it = _entries.begin(); it != _entries.end(); ++it )
			file << it->first << "=" << it->second << std::endl;

		file << std::endl;
	}

	IniFile::IniFile( const std::string &filename ) :
		_filename( filename )
	{}

	IniFile::~IniFile()
	{
		for( auto it = _sections.begin(); it != _sections.end(); ++it )
			safeDelete( it->second );
		_sections.clear();
	}

	bool IniFile::read()
	{
		if( !std::filesystem::exists( _filename ) )
		{
			//file not yet created -- not an error though
			std::cerr << "<warning> file " << _filename << " does not exists (yet?)" << std::endl;
			return true;
		}

		std::fstream file( _filename, std::ios::in );
		if( !file.is_open() )
		{
			std::cerr << "<error> could not open file " << _filename << " for reading" << std::endl;
			return false;
		}

		int cntr = 0;
		std::string line;
		Section *currentSection = nullptr;
		for( std::string line; std::getline( file, line ); cntr++ )
		{
			trim( line );

			if( line.empty() )
				continue;

			if( line.front() == '[' )
			{
				if( line.back() != ']' || line.size() < 3 )
				{
					std::cerr << "<error> parsing error in file " << _filename << " line #" << cntr << std::endl;
					return false;
				}

				std::string name( line.begin() + 1, line.end() - 1 );
				currentSection = new Section( name );
				_sections.insert( std::make_pair( name, currentSection ) );

				continue;
			}
			
			if( !currentSection )
			{
				std::cerr << "<error> parsing error in file " << _filename << " line #" << cntr << " (no section specified)" << std::endl;
				return false;
			}

			std::vector<std::string> subs = split( line, '=' );
			if( subs.size() != 2 )
			{
				std::cerr << "<error> parsing error in file " << _filename << " line #" << cntr << std::endl;
				return false;
			}

			std::string &key = subs[0];
			std::string &value = subs[1];

			currentSection->set( key, value );
		}

		return true;
	}

	bool IniFile::write()
	{
		std::fstream file( _filename, std::ios::out | std::ios::trunc );
		if( !file.is_open() )
		{
			std::cerr << "<error> could not open file " << _filename << " for writing" << std::endl;
			return false;
		}

		for( auto it = _sections.begin(); it != _sections.end(); ++it )
			it->second->write( file );

		return true;
	}

	template<>
	std::string IniFile::Section::convert<std::string>( const std::string &s ) const
	{
		return s;
	}

	template<>
	bool IniFile::Section::convert<bool>( const std::string &s ) const
	{
		std::string str( s );
		trim( str );

		if( !_stricmp( s.c_str(), "false" ) || !_stricmp( s.c_str(), "0" ) || !_stricmp( s.c_str(), "low" ) )
			return false;
		else if( !_stricmp( s.c_str(), "true" ) || !_stricmp( s.c_str(), "1" ) || !_stricmp( s.c_str(), "high" ) )
			return true;

		throw std::runtime_error( "could not parse to bool value" );
	}

	template<>
	glm::mat4 IniFile::Section::get( const std::string &name ) const
	{
		glm::mat4 m = glm::identity<glm::mat4>();

		auto it = _entries.find( name );
		if( it == _entries.end() )
			return m;

		std::string str( it->second );
		trim( str );

		std::vector<std::string> values = split( str, ',' );
		if( values.size() < 16 )
			return m;

		float *v = glm::value_ptr( m );
		for( int i = 0; i < 16; i++ )
			*( v++ ) = atof( values[i].c_str() );

		return m;
	}

	template<>
	SampleFrame IniFile::Section::get( const std::string &name ) const
	{
		SampleFrame t;

		auto it = _entries.find( name );
		if( it == _entries.end() )
			return t;

		std::string str( it->second );
		trim( str );

		float *f = nullptr;
		std::vector<std::string> planes = split( str, '|' );
		for( int k = 0; k < planes.size(); k++ )
		{
			std::vector<std::string> lines = split( planes[k], ';' );
			for( int j = 0; j < lines.size(); j++ )
			{
				std::vector<std::string> cols = split( lines[j], ',' );
				for( int i = 0; i < cols.size(); i++ )
				{
					if( !f )
					{
						t.alloc( cols.size(), lines.size(), planes.size() );
						f = t.values();
					}

					*( f++ ) = atof( cols[i].c_str() );
				}
			}
		}

		return t;
	}

	template<>
	bool IniFile::Section::tryGet<glm::mat4>( const std::string &name, glm::mat4 &m ) const
	{
		auto it = _entries.find( name );
		if( it == _entries.end() )
			return false;

		std::string str( it->second );
		trim( str );

		std::vector<std::string> values = split( str, ',' );
		if( values.size() < 16 )
			return false;

		float *v = glm::value_ptr( m );
		for( int i = 0; i < 16; i++ )
			*( v++ ) = atof( values[i].c_str() );

		return true;
	}

	template<>
	bool IniFile::Section::tryGet<SampleFrame>( const std::string &name, SampleFrame &t ) const
	{
		auto it = _entries.find( name );
		if( it == _entries.end() )
			return false;

		std::string str( it->second );
		trim( str );

		float *f = nullptr;
		std::vector<std::string> planes = split( str, '|' );
		for( int k = 0; k < planes.size(); k++ )
		{
			std::vector<std::string> lines = split( planes[k], ';' );
			for( int j = 0; j < lines.size(); j++ )
			{
				std::vector<std::string> cols = split( lines[j], ',' );
				for( int i = 0; i < cols.size(); i++ )
				{
					if( !f )
					{
						t.alloc( cols.size(), lines.size(), planes.size() );
						f = t.values();
					}

					*( f++ ) = atof( cols[i].c_str() );
				}
			}
		}

		return true;
	}

	template<>
	void IniFile::Section::set<glm::mat4>( const std::string &name, const glm::mat4 &m )
	{
		const float *v = glm::value_ptr( m );
		std::stringstream sstr;
		for( int i = 0; i < 16; i++ )
			sstr << *( v++ ) << ",";

		_entries[name] = sstr.str();
	}

	template<>
	void IniFile::Section::set<SampleFrame>( const std::string &name, const SampleFrame &t )
	{
		int cntr = 0;
		std::stringstream sstr;
		for( int k = 0; k < t.depth(); k++ )
		{
			for( int j = 0; j < t.height(); j++ )
			{
				for( int i = 0; i < t.width(); i++ )
					sstr << t.values()[cntr++] << ",";
				sstr << ";";
			}
			sstr << "|";
		}

		_entries[name] = sstr.str();
	}
}