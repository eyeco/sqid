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


#include "fileWriter.h"

namespace sqid
{
	class CSVWriter : public FileWriter
	{
	private:
		const char *_delimiter;

	protected:
		virtual bool isBinary() { return false; }

		virtual bool insert( const float *measurements, size_t count );

	public:
		explicit CSVWriter( const std::string &filename, bool append = false );
		virtual ~CSVWriter();

		virtual bool writeFrame( const SampleFrame *sf );
	};
}
