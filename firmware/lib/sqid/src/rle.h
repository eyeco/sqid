/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#ifndef _SQID_RLE
#define _SQID_RLE

#include "common.h"

#include "stddef.h"

namespace sqid
{
#ifdef _SQID_RLE_ENC_SUPPORT
    size_t encodeRLE( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax );
    size_t decompressRLE( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax );
#endif
}

#endif