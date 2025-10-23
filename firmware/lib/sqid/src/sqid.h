/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#ifndef _SQID
#define _SQID

#include <stddef.h>

namespace sqid
{
    enum Layout
    {
        L_0D,
        L_1D,
        L_2D,
        L_3D,

        L_COUNT
    };

    enum DataType
    {
        DT_BYTE,
        DT_USHORT,
        DT_ULONG,
        DT_FLOAT,

        DT_COUNT
    };

    inline Layout getLayout( size_t width, size_t height, size_t depth )
    {
        if(!(width * height * depth))
            return L_COUNT;

        if(depth == 1)
        {
            if(height == 1)
            {
                if(width == 1)
                    return L_0D;
                else
                    return L_1D;
            }
            else
                return L_2D;
        }
        else
            return L_3D;
    }
}

#endif