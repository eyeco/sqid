/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "csvWriter.h"

#include <sampleFrame.h>

#include <iostream>

namespace sqid
{
	CSVWriter::CSVWriter( const std::string &filename, bool append ) :
		FileWriter( filename, append ),
		_delimiter( "," )
	{}

	CSVWriter::~CSVWriter()
	{}

	bool CSVWriter::insert( const float *measurements, size_t count )
	{
		if( _file.is_open() )
		{
			std::cerr << "<error> csv file is not open" << std::endl;
			return false;
		}

		for( int i = 0; i < count; i++ )
			_file << *( measurements++ ) << _delimiter;
		_file << std::endl;

		return true;
	}

	bool CSVWriter::writeFrame( const SampleFrame *sf )
	{
		if( !_file.is_open() )
			return false;

		_file << sf->width() << _delimiter;
		_file << sf->height() << _delimiter;
		_file << sf->depth() << _delimiter;
		_file << sf->timeStamp() << _delimiter;

		const float *f = sf->values();
		for( int i = 0; i < sf->size(); i++ )
			_file << *( f++ ) << _delimiter;
		_file << std::endl;

		return true;
	}
}
