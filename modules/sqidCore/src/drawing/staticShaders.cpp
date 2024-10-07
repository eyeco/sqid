/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "staticShaders.h"

#ifdef __SUPPORT_GUI

namespace sqid
{
#ifdef __STATIC_SHADERS
	
	//vertex shaders
	const char* vertSourcePassthrough =
#include "../../../../resources/shaders/include/passthrough.vert.h"
		;


	//fragment shaders
	const char* fragSourcePassthrough =
#include "../../../../resources/shaders/include/passthrough.frag.h"
		;
	const char* fragSourceFont =
#include "../../../../resources/shaders/include/font.frag.h"
		;
	const char* fragSourceSingleChannel =
#include "../../../../resources/shaders/include/multiChannel.frag.h"
		;
	const char* fragSourceMultiChannel =
#include "../../../../resources/shaders/include/singleChannel.frag.h"
		;
#endif
}

#endif