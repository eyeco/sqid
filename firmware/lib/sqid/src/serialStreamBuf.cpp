/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#include "serialStreamBuf.h"

#include "transport.h"

#include "stdio.h"

namespace sqid
{
    int SerialStreamBuf::overflow(int c)
    {
        if (c == EOF)
        {
            return !EOF;
        }
        else
        {
            uint8_t cc = c;
            TransportSerial::singleton().write(&cc, 1);
            return c;
        }
    }
}