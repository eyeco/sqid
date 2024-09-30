/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


/*
#include <fileIO/matWriter.h>
#include <sampleFrame.h>

#include <time.h>
#include <iostream>

namespace sqid
{
	MatWriterAscii::MatWriterAscii() :
		FileWriter(),
		octaveStyle( !writeImmediate ),
		delimiter( " " )
	{}

	MatWriterAscii::MatWriterAscii( const std::string &filename, unsigned int columns, bool writeImmediate, bool octaveStyle ) :
		FileWriter( filename ),
		octaveStyle( octaveStyle ),
		writeImmediate( writeImmediate ),
		columns( columns ),
		delimiter( " " )
	{
		this->setup();
	}

	MatWriterAscii::MatWriterAscii( const std::string &filename, const std::string &matName, unsigned int columns, bool writeImmediate, bool octaveStyle ) :
		FileWriter( filename ),
		octaveStyle( octaveStyle ),
		writeImmediate( writeImmediate ),
		columns( columns ),
		delimiter( " " )
	{
		this->setup();
	}

	MatWriterAscii::~MatWriterAscii()
	{
		this->close();
	}

	bool MatWriterAscii::writeFrame( const SampleFrame *sf )
	{
		if( sf->depth() != 1 )
			throw std::runtime_error( "3D matrices not supported for MATLAB ASCII files" );

		if( sf->depth() != 1 )
			throw std::runtime_error( "3D matrices not yet implemented for MATLAB files" );

		std::vector<float> values( numHeaderValues + sf->size() );
		float *f = &values[0];

		*( f++ ) = sf->width();
		*( f++ ) = sf->height();
		*( f++ ) = sf->depth();
		*( f++ ) = sf->timeStamp();

		std::copy( sf->values(), sf->values() + sf->size(), f );

		return this->insert( &values[0], values.size() );
	}

	void MatWriterAscii::writeData( const float *data, size_t count )
	{
		if( !this->file.is_open() )
		{
			std::cerr << "<error> mat file is not open" << std::endl;
			return;
		}

		for( unsigned int i = 0; i < count; i++ )
			this->file << this->delimiter << *( data++ );
		this->file << std::endl;
	}

	void MatWriterAscii::writeHeader()
	{
		//in MATLAB ASCII format, there is no header at all
		if( this->octaveStyle )
		{
			time_t rawTime;
			time( &rawTime );
			struct tm *timeinfo = localtime( &rawTime );

			char timeBuffer[256];
			strftime( timeBuffer, sizeof( timeBuffer ), "%a %b %d %H:%M:%S %Y %Z", timeinfo );

			size_t rows = this->values.size() / ( this->columns + numHeaderValues );

			this->file << "% Created by vaderlib, " << timeBuffer << std::endl;
			if( this->matName.size() )
				this->file << "% name: " << this->matName << std::endl;
			this->file << "% type: matrix" << std::endl;
			this->file << "% rows: " << rows << std::endl;
			this->file << "% columns: " << columns + numHeaderValues << std::endl;
		}
	}

	bool MatWriterAscii::setup()
	{
		this->close();

		if( this->matName.size() && !this->octaveStyle )
			std::cerr << "<warning> matrix name will be ignored in MATLAB file format" << std::endl;
		if( this->writeImmediate && this->octaveStyle )
		{
			//Octave header needs to know matrix size, so we can't write file on-the-fly
			std::cerr << "<warning> immediate matrix writing mode not supported for Octave files -- switching to delayed writing" << std::endl;
			this->writeImmediate = false;
		}

		this->delimiter = ( this->octaveStyle ? " " : "   " );

		if( this->writeImmediate )
		{
			if( !this->open() )
				return false;
			this->writeHeader();
		}

		return true;
	}

	bool MatWriterAscii::open()
	{
		if( !FileWriter::open() )
			return false;

		if( this->octaveStyle )
		{
			this->file.setf( std::ios::fixed );
			this->file.precision( 16 );
		}
		else
		{
			this->file.setf( std::ios::scientific );
			this->file.precision( 7 );
		}

		return true;
	}

	bool MatWriterAscii::close()
	{
		if( !this->columns )	//not yet set up
			return true;

		if( !this->file.is_open() )	//already closed?
			return true;

		if( !this->writeImmediate )
		{
			if( !this->open() )
				return false;

			size_t rows = this->values.size() / this->columns;

			const float *dataPtr = &this->values[0];
			for( unsigned int j = 0; j < rows; j++ )
			{
				this->writeData( dataPtr, this->columns );
				dataPtr += this->columns;
			}
		}

		this->file.flush();
		this->file.close();

		this->values.clear();

		return true;
	}

	bool MatWriterAscii::insert( const float *measurements, size_t count )
	{
		if( this->writeImmediate )
			this->writeData( measurements, count );
		else
		{
			if( !this->columns )
			{
				std::cerr << "<error> MatWriter not properly set up" << std::endl;
				return false;
			}

			if( count % this->columns )
			{
				std::cerr << "<error> invalid number of values passed to MatWriter" << std::endl;
				return false;
			}

			this->values.insert( this->values.end(), measurements, measurements + count );
		}

		return true;
	}





	MatWriterBinary::MatWriterBinary() :
		FileWriter(),
		rows( 0 )
	{
		this->setup();
	}

	MatWriterBinary::MatWriterBinary( const std::string &filename, unsigned int columns, unsigned int rows ) :
		FileWriter( filename ),
		rows( rows ),
		columns( columns )
	{
		this->setup();
	}

	MatWriterBinary::~MatWriterBinary()
	{
		this->close();
	}

	bool MatWriterBinary::writeFrame( const SampleFrame *sf )
	{
		return false;
	}

	void MatWriterBinary::writeData( const float *data, size_t count )
	{
		if( !this->file.is_open() )
		{
			std::cerr << "<error> mat file is not open" << std::endl;
			return;
		}

		//TODO
		throw std::runtime_error( "not implemented" );
	}

	void MatWriterBinary::writeHeader()
	{
		//TODO
		throw std::runtime_error( "not implemented" );
	}

	bool MatWriterBinary::setup()
	{
		return false;
	}

	bool MatWriterBinary::close()
	{
		return false;
	}
}
*/