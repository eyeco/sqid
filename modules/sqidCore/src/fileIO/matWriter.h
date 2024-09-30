/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once
/*
#include "fileWriter.h"

#include <vector>

namespace sqid
{
	
	class MatWriterAscii : public FileWriter
	{
	private:
		//octave writes header with 
		// - creation info (library/application + date/time)
		// - data type 
		// - name of data container (aka matrix name in octave workspace)
		// - matrix dimensions -> hence, octaveStyle matrices cannot be written in immediate mode, as row count has to be known at header-writing-time
		bool octaveStyle;

		bool writeImmediate;
		unsigned int columns;

		std::vector<float>	values;

		const char *delimiter;
		std::string matName;

		virtual bool open();
		virtual bool insert( const float *measurements, size_t count );

	protected:
		virtual bool isBinary() { return false; }

		void writeData( const float *data, size_t count );
		void writeHeader();

	public:
		MatWriterAscii();
		MatWriterAscii( const std::string &filename, unsigned int columns, bool writeImmediate, bool octaveStyle = false );
		MatWriterAscii( const std::string &filename, const std::string &matName, unsigned int columns, bool writeImmediate, bool octaveStyle = false );
		virtual ~MatWriterAscii();

		bool setup();

		virtual bool close();
		virtual bool writeFrame( const SampleFrame *sf );
	};

	//https://www.mathworks.com/help/pdf_doc/matlab/matfile_format.pdf
	class MatWriterBinary : public FileWriter
	{
	private:
		enum DataType
		{
			DT_INT8 = 1, // 8 bit, signed
			DT_UINT8 = 2, // 8 bit, unsigned
			DT_INT16 = 3, // 16-bit, signed
			DT_UINT16 = 4, // 16-bit, unsigned
			DT_INT32 = 5, // 32-bit, signed
			DT_UINT32 = 6, // 32-bit, unsigned
			DT_SINGLE = 7, // IEEE® 754 single format
			// reserved
			DT_DOUBLE = 9, // IEEE 754 double format
			// reserved
			// reserved
			DT_INT64 = 12, // 64-bit, signed
			DT_UINT64 = 13, // 64-bit, unsigned
			DT_MATRIX = 14, // MATLAB array
			DT_COMPRESSED = 15, // Compressed Data
			DT_UTF8 = 16, // Unicode UTF-8 Encoded Character Data
			DT_UTF16 = 17, // Unicode UTF-16 Encoded Character Data
			DT_UTF32 = 18 // Unicode UTF-32 Encoded Character Data
		};

		unsigned int rows;
		unsigned int columns;

		std::vector<float>	values;

	protected:
		virtual bool isBinary() { return true; }

		void writeData( const float *data, size_t count );
		void writeHeader();

	public:
		MatWriterBinary();
		MatWriterBinary( const std::string &filename, unsigned int columns, unsigned int rows = 0 );
		virtual ~MatWriterBinary();

		bool setup();

		virtual bool close();
		virtual bool writeFrame( const SampleFrame *sf );
	};
}
*/