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

#include "fileWriter.h"

namespace sqid
{
	class BinWriter : public FileWriter
	{
	protected:
		virtual bool isBinary() { return true; }

		virtual bool insert( const float *values, size_t count );

	public:
		explicit BinWriter( const std::string &filename, bool append = false );
		virtual ~BinWriter();

		virtual bool writeFrame( const SampleFrame *sf );
	};
}
