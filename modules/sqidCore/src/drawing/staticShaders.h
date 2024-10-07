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

#include <config.h>

#ifdef __SUPPORT_GUI

namespace sqid
{
#ifdef __STATIC_SHADERS
	//vertex shaders
	extern const char* vertSourcePassthrough;

	//fragment shaders
	extern const char* fragSourcePassthrough;
	extern const char* fragSourceFont;
	extern const char* fragSourceSingleChannel;
	extern const char* fragSourceMultiChannel;
#endif
}

#endif