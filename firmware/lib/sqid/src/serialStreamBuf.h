/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#ifndef _SQID_SERIAL_STREAM_BUF
#define _SQID_SERIAL_STREAM_BUF

#include <iostream>

namespace sqid
{
    class SerialStreamBuf : public std::streambuf
    {
    private:
        virtual int overflow(int c);
    };

    inline void redirect( std::ostream &ostr, SerialStreamBuf &buf )
    {
        ostr.rdbuf(&buf);
    }
}

#endif