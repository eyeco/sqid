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

#include <fstream>

namespace sqid
{
	class SampleFrame;

	class FileWriter
	{
	protected:
		static unsigned int numHeaderValues;

		bool _isOpen;
		bool _append;

		std::fstream _file;
		std::string _filename;

		virtual bool isBinary() = 0;
		virtual bool insert( const float *measurements, size_t count ) = 0;

		bool insert( const std::vector<float> &measurements );

	public:
		explicit FileWriter( const std::string &filename, bool append = false );
		virtual ~FileWriter();

		bool open();
		bool close();

		virtual bool writeFrame( const SampleFrame *sf ) = 0;
	};
}
