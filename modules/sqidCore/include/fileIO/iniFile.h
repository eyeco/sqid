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

#include <common.h>
#include <sampleFrame.h>

#include <map>
#include <string>
#include <sstream>
#include <fstream>

namespace sqid
{
	class SQID_API IniFile
	{
	public:
		class Section
		{
			friend class IniFile;

		private:
			std::string _name;
			std::map<std::string, std::string> _entries;

			void write( std::fstream &file );

			template<typename T>
			T convert( const std::string &s ) const
			{
				std::stringstream sstr( s );

				T value = 0;
				sstr >> value;

				return value;
			}

		public:
			explicit Section( const std::string &name ) :
				_name( name )
			{}

			template<typename T>
			T get( const std::string &name ) const
			{
				auto it = _entries.find( name );
				if( it == _entries.end() )
					return 0;

				return convert<T>( it->second );
			}

			template<typename T>
			bool tryGet( const std::string &name, T &t ) const
			{
				auto it = _entries.find( name );
				if( it == _entries.end() )
					return false;

				t = convert<T>( it->second );
				return true;
			}

			template<typename T>
			void set( const std::string &name, const T &t )
			{
				std::stringstream sstr;
				sstr << t;

				_entries[name] = sstr.str();
			}
		};

	private:
		std::string _filename;
		std::map<std::string, IniFile::Section*> _sections;

	public:
		explicit IniFile( const std::string &filename );
		~IniFile();

		bool read();
		bool write();

		IniFile::Section *get( const std::string &name )
		{
			auto it = _sections.find( name );
			if( it == _sections.end() )
				it = _sections.insert( std::make_pair( name, new IniFile::Section( name ) ) ).first;
			return it->second;
		}

		IniFile::Section *tryGet( const std::string &name ) const
		{
			auto it = _sections.find( name );
			if( it == _sections.end() )
				return nullptr;
			return it->second;
		}

		IniFile::Section &operator[]( const std::string &name )
		{
			return *this->get( name );
		}
	};

	template<> std::string IniFile::Section::convert<std::string>( const std::string &s ) const;
	template<> bool IniFile::Section::convert<bool>( const std::string &s ) const;
	template<> glm::mat4 IniFile::Section::get( const std::string &name ) const;
	template<> SampleFrame IniFile::Section::get( const std::string &name ) const;
	template<> bool IniFile::Section::tryGet<glm::mat4>( const std::string &name, glm::mat4 &m ) const;
	template<> bool IniFile::Section::tryGet<SampleFrame>( const std::string &name, SampleFrame &t ) const;
	template<> void IniFile::Section::set<glm::mat4>( const std::string &name, const glm::mat4 &m );
	template<> void IniFile::Section::set<SampleFrame>( const std::string &name, const SampleFrame &t );
}