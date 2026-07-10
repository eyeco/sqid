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

#include <glm/glm.hpp>

namespace sqid
{
	class Font;

	class SQID_API UIStyle
	{
	public:
		const unsigned int FinderWidth;

		const glm::vec3 BackgroundColor;
		const glm::vec3 GridColor;

		const glm::vec3 BackgroundColorEdit;
		const glm::vec3 GridColorEdit;

		const glm::vec3 SelectionLineColor;
		const glm::vec3 SelectionFillColor;
		const float SelectionFillOpacity;

		const float IconLineWidth;
		const float ButtonLineWidth;
		const float ConnectorLineWidth;

		const glm::vec3 RubberColor;
		const float RubberLineWidth;

		const glm::vec3 SourceFeedLineColor;
		const glm::vec3 SinkFeedLineColor;
		const glm::vec3 ConnectorLineColor;

		const int NodePaddingX;
		const int NodePaddingY;

		const int NodeMapDY;

		const int NodeMapWidth;

		const int NodeMinWidth;
		const int NodeMinHeight;

		const int NodePinWidth;
		const int NodePinDist;

		const int NodeButtonWidth;

		const int PinPadding;

		const int InterfaceNodeWidth;
		const int InterfaceNodeHeight;

		const int TextDY;
		const int TextSize;

		const glm::vec3 TextColor;

		const float NodeLineWidth;

		const glm::vec3 NodeColorBG;

		const glm::vec3 NodeColorHoveredBG;
		const glm::vec3 NodeColorDisabledBG;

		const glm::vec3 NodeColorDraggingBG;

		const float NodeSelectionLineWidth;
		const float NodeSelectionLineDist;
		const glm::vec3 NodeSelectionLineColor;

		const glm::vec3 PinColor;
		const glm::vec3 PinColorHovered;
		const glm::vec3 PinColorActivity;
		const glm::vec3 PinColorCompatible;

		const glm::vec3 ButtonColorBG;

		const glm::vec3 ButtonColorActiveBG;

		const glm::vec3 ButtonColorPressedBG;

	private:
		Font *_font;

	public:
		UIStyle();
		~UIStyle();

		void init();

		Font &getFont() const;
	};
}
#endif