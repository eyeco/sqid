/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <commonImGui.h>

//#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace sqid
{
#ifdef __SUPPORT_GUI
	ScopedImGuiDisable::ScopedImGuiDisable( bool disable ) :
		_disabled( false )
	{
		if( disable )
			this->disable();
	}

	ScopedImGuiDisable::~ScopedImGuiDisable()
	{
		enable();
	}

	void ScopedImGuiDisable::disable()
	{
		if( !_disabled )
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );

			_disabled = true;
		}
	}

	void ScopedImGuiDisable::enable()
	{
		if( _disabled )
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();

			_disabled = false;
		}
	}


	ScopedImGuiStyleColor::ScopedImGuiStyleColor() :
		_pushed( false )
	{}

	ScopedImGuiStyleColor::ScopedImGuiStyleColor( ImGuiCol idx, const ImVec4& col ) :
		_pushed( false )
	{
		set( idx, col );
	}

	ScopedImGuiStyleColor::~ScopedImGuiStyleColor()
	{
		reset();
	}

	void ScopedImGuiStyleColor::set( ImGuiCol idx, const ImVec4& col )
	{
		reset();

		ImGui::PushStyleColor( idx, col );
		_pushed = true;
	}

	void ScopedImGuiStyleColor::reset()
	{
		if( _pushed )
			ImGui::PopStyleColor();
	}
#endif
}