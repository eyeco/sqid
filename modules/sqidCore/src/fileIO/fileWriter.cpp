/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "fileWriter.h"

namespace sqid
{
	unsigned int FileWriter::numHeaderValues = 4;

	FileWriter::FileWriter( const std::string &filename, bool append ) :
		_isOpen( false ),
		_append( append ),
		_filename( filename )
	{}

	FileWriter::~FileWriter()
	{
		close();
	}

	bool FileWriter::open()
	{
		std::ios::openmode flags = std::ios::out;
		
		if( isBinary() )
			flags |= std::ios::binary;

		if( _append )
			flags |= std::ios::app;
		else
			flags |= std::ios::trunc;
		
		_file.open( _filename, flags );
		if( !_file.is_open() )
		{
			std::cerr << "<error> failed opening files " << _filename << " for writing" << std::endl;
			return false;
		}

		return true;
	}

	bool FileWriter::close()
	{
		if( !_file.is_open() )	//already closed?
			return true;

		_file.flush();
		_file.close();

		return true;
	}

	bool FileWriter::insert( const std::vector<float> &measurements )
	{
		return insert( &measurements[0], measurements.size() );
	}
}
