/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "uiStyle.h"

#ifdef __SUPPORT_GUI

#include "../font.h"

#include <common.h>

namespace sqid
{
	UIStyle::UIStyle() :
		FinderWidth( 300 ),

		BackgroundColor( fromHex( 0xcfcfcf ) ),
		GridColor( fromHex( 0xc5c5c5 ) ),

		BackgroundColorEdit( fromHex( 0x303030 ) ),
		GridColorEdit( fromHex( 0x252525 ) ),

		SelectionLineColor( fromHex( 0x40407f ) ),
		SelectionFillColor( fromHex( 0x20207f ) ),
		SelectionFillOpacity( 0.5f ),

		IconLineWidth( 1.0f ),
		ButtonLineWidth( 1.0f ),
		ConnectorLineWidth( 2.0f ),

		RubberColor( fromHex( 0xffff00 ) ),
		RubberLineWidth( 3.0f ),

		SourceFeedLineColor( fromHex( 0x9d1931 ) ),
		SinkFeedLineColor( fromHex( 0xe3d400 ) ),
		ConnectorLineColor( fromHex( 0x3d85e0 ) ),

		NodePaddingX( 15 ),
		NodePaddingY( 10 ),

		NodeMapDY( 15 + 12 + 10 ),

		NodeMapWidth( 200 ),

		NodeMinWidth( NodePaddingX * 2 + NodeMapWidth ),
		NodeMinHeight( NodeMapDY + NodePaddingY ),

		NodePinWidth( 15 ),
		NodePinDist( 45 ),

		NodeButtonWidth( 16 ),

		PinPadding( 30 ),

		InterfaceNodeWidth( NodeMinWidth ),
		InterfaceNodeHeight( NodeMinHeight ),

		TextDY( NodePaddingY * 1.5f ),
		TextSize( 12 ),

		TextColor( fromHex( 0x000000 ) ),

		NodeLineWidth( 2.0f ),

		NodeColorBG( fromHex( 0xa7a7a7 ) ),// 0xe7e7e7 ) ) ),

		NodeColorDisabledBG( NodeColorBG * 0.5f ),// 0xe7e7e7 ) ) ),
		NodeColorHoveredBG( NodeColorBG * 1.2f ),
		NodeColorDraggingBG( fromHex( 0xffff00 ) ),

		NodeSelectionLineWidth( 5.0f ),
		NodeSelectionLineDist( 5.0f ),
		NodeSelectionLineColor( fromHex( 0x00ff00 ) ),

		PinColor( fromHex( 0x3d85e0 ) ),
		PinColorHovered( fromHex( 0x23466e ) ),
		PinColorActivity( fromHex( 0xff0000 ) ),
		PinColorCompatible( fromHex( 0x00ff00 ) ),

		ButtonColorBG( fromHex( 0x7797b7 ) ),
		ButtonColorActiveBG( fromHex( 0x3d85e0 ) ),
		ButtonColorPressedBG( ButtonColorBG * 0.5f ),

		_font( nullptr )
	{}

	UIStyle::~UIStyle()
	{
		safeDelete( _font );
	}

	void UIStyle::init()
	{
		_font = new Font( "resources/fonts/arial.ttf", TextSize );
	}

	Font &UIStyle::getFont() const
	{	
		return *_font;
	}
}
#endif