/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "binWriter.h"

#include <sampleFrame.h>

#include <iostream>

namespace sqid
{
	BinWriter::BinWriter( const std::string &filename, bool append ) :
		FileWriter( filename, append )
	{}

	BinWriter::~BinWriter()
	{}

	bool BinWriter::insert( const float *measurements, size_t count )
	{
		if( _file.is_open() )
		{
			std::cerr << "<error> bin file is not open" << std::endl;
			return false;
		}

		_file.write( (const char*)measurements, sizeof( float ) * count );

		return true;
	}

	template<typename T>
	void write( const T &t, std::fstream &file )
	{
		file.write( (char*)&t, sizeof( T ) );
	}

	bool BinWriter::writeFrame( const SampleFrame *sf )
	{
		if( !_file.is_open() )
			return false;

		write<size_t>( sf->width(), _file );
		write<size_t>( sf->height(), _file );
		write<size_t>( sf->depth(), _file );
		write<uint32_t>( sf->timeStamp(), _file );

		_file.write( (char*) &sf->values()[0], sizeof( float ) * sf->size() );

		return true;
	}
}
