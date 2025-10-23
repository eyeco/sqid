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

#include <config.h>

#ifdef __SUPPORT_GUI
#include <common.h>
#include <commonGL.h>

namespace sqid
{
	class SQID_API FrameBuffer
	{
	private:
		std::string _name;

		bool _valid;

		GLuint _rtName;
		GLuint _fboID;

		//intentionally no implemented
		//prevent from copying via copy constructor -- have to use reference counting to not end up with invalid gl FBO IDs
		FrameBuffer( const FrameBuffer& );

	public:
		explicit FrameBuffer( const std::string &name );
		~FrameBuffer();

		bool build( size_t width, size_t height, GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR );
		void destroy();

		bool activate();
		bool deactivate();

		GLuint getRenderTextureName() const { return _rtName; }

		const std::string &getName() const { return _name; }
	};
}
#endif