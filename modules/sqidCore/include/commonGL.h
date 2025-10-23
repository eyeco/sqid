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

#include <GL/glew.h>

//#ifdef _WIN32
//#include <gl/wglew.h>
//#endif

namespace sqid
{
#ifdef __SUPPORT_GUI
	class SQID_API ScopedGlAttribs
	{
	public:
		ScopedGlAttribs( GLbitfield bits )
		{
			glPushAttrib( bits );
		}

		~ScopedGlAttribs()
		{
			glPopAttrib();
		}
	};
#endif
}