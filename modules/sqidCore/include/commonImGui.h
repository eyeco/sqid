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

#include <imgui/imgui.h>

namespace sqid
{
	inline ImVec2 glm2im( const glm::ivec2 &v )
	{
		return ImVec2( (float) v.x, (float) v.y );
	}

	inline ImVec4 glm2im( const glm::ivec4 &v )
	{
		return ImVec4( (float) v.x, (float) v.y, (float) v.z, (float) v.w );
	}

	inline ImVec2 glm2im( const glm::vec2 &v )
	{
		return ImVec2( v.x, v.y );
	}

	inline ImVec4 glm2im( const glm::vec4 &v )
	{
		return ImVec4( v.x, v.y, v.z, v.w );
	}

	inline glm::vec2 im2glm( const ImVec2 &v )
	{
		return glm::vec2( v.x, v.y );
	}

	inline glm::vec4 im2glm( const ImVec4 &v )
	{
		return glm::vec4( v.x, v.y, v.z, v.w );
	}

#ifdef __SUPPORT_GUI
	class SQID_API ScopedImGuiDisable
	{
	private:
		bool _disabled;

	public:
		explicit ScopedImGuiDisable( bool disable = true );
		~ScopedImGuiDisable();

		void disable();
		void enable();
	};

	class SQID_API ScopedImGuiStyleColor
	{
	private:
		bool _pushed;

	public:
		ScopedImGuiStyleColor();
		ScopedImGuiStyleColor( ImGuiCol idx, const ImVec4& col, bool use = true );
		~ScopedImGuiStyleColor();

		void set( ImGuiCol idx, const ImVec4& col );
		void reset();
	};
#endif
}