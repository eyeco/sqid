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

#include <processing/pin.h>

#include "guiElement.h"

namespace sqid
{
	class PinDrawer : 
		public GUI::Element,
		public GUI::Hoverable
	{
	public:
		enum Event
		{
			E_PRESSED,
			E_RELEASED,

			E_COUNT
		};

		typedef std::function<void( PinDrawer*, PinDrawer::Event )> PinDrawerCallback;

	protected:
		bool _drawCompatible;

		PinDrawerCallback _cb;

	public:
		explicit PinDrawer( GUI::Drawer *drawer );
		~PinDrawer();

		virtual bool draw();

		virtual bool drawTooltip();

		virtual bool mouseDown( int button, int mods, bool imGuiHandled );
		virtual bool mouseUp( int button, int mods, bool imGuiHandled );

		virtual bool isInlet() const { return false; }
		virtual bool isOutlet() const { return false; }

		virtual Pin *getPin() const = 0;

		void updateCompatibility( const PinDrawer *outlet );

		void setCallback( PinDrawerCallback cb, void *usrPtr = nullptr ) { _cb = cb; }
	};

	class InletPinDrawer : public PinDrawer
	{
	private:
		InletPin *_pin;

	public:
		InletPinDrawer( GUI::Drawer *drawer, InletPin *pin );

		virtual Pin *getPin() const	{ return _pin; }
		InletPin *getInlet() const { return _pin; }

		virtual bool isInlet() const { return true; }
	};

	class OutletPinDrawer : public PinDrawer
	{
	private:
		OutletPin *_pin;

	public:
		OutletPinDrawer( GUI::Drawer *drawer, OutletPin *pin );

		virtual Pin *getPin() const { return _pin; }
		OutletPin *getOutlet() const { return _pin; }

		virtual bool isOutlet() const { return true; }
	};
}
#endif