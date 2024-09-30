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

#include <functional>

#include "guiElement.h"

namespace sqid
{
	namespace GUI
	{
		class Icon;

		class Button :
			public GUI::Element,
			public GUI::Hoverable,
			public GUI::Pressable
		{
		public:
			enum Event
			{
				E_PRESSED,
				E_RELEASED,

				E_COUNT
			};

			typedef std::function<void( const Button*, Button::Event )> ButtonCallback;

		private:
			bool _toggle;
			bool _active;

			Icon *_icon;
			Icon *_iconActive;

			ButtonCallback _cb;

		public:
			Button( Drawer *drawer, bool toggle, Icon *icon = nullptr, Icon *iconActive = nullptr );	//button takes ownership over icons
			~Button();

			virtual bool draw();

			virtual bool mouseDown( int button, int mods, bool imGuiHandled );
			virtual bool mouseUp( int button, int mods, bool imGuiHandled );

			virtual float setZ( float z );

			bool getActive() const { return _active; }
			void setActive( bool active );

			void setCallback( ButtonCallback cb ) { _cb = cb; }
		};
	}
}

#endif
