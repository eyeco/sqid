/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "guiDrawer.h"

#ifdef __SUPPORT_GUI

#include <app.h>
#include "guiElement.h"

namespace sqid
{
	namespace GUI
	{
		Drawer::Drawer( const UIStyle *uis ) :
			_uis( uis )
		{}

		Drawer::~Drawer()
		{
			_elements.clear();

			_uis = nullptr;
		}

		void Drawer::add( Element *e )
		{
			_elements.push_back( e );
		}

		void Drawer::remove( Element *e )
		{
			_elements.remove( e );
		}

		const glm::ivec2 &Drawer::getWindowSize() const
		{
			return App().getWindowSize();
		}

		const glm::mat4 &Drawer::getMV() const
		{
			return App().getMVCanvas();
		}
	}
}

#endif