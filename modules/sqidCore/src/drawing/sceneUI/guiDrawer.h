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

#include <list>
#include <string>

#include <common.h>

namespace sqid
{
	class UIStyle;

	namespace GUI
	{
		class Element;

		class SQID_API Drawer
		{
		protected:
			std::list<Element*> _elements;

			const UIStyle* _uis;

		public:
			explicit Drawer( const UIStyle *uis );
			virtual ~Drawer();

			void add( Element *e );		//does NOT take ownership
			void remove( Element *e );	//does NOT delete

			const glm::ivec2 &getWindowSize() const;
			const glm::mat4 &getMV() const;

			const UIStyle* getUIStyle() const { return _uis; }
		};
	}
}

#endif
