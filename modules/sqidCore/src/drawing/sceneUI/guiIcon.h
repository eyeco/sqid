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

#include <map>
#include <vector>

#include "guiElement.h"

namespace sqid
{
	namespace GUI
	{
		class Icon : public Element
		{
		protected:
			std::vector<glm::vec2> _verts;
			std::vector<glm::vec3> _cols;

		public:
			Icon( Drawer *drawer, float size );
			virtual ~Icon();
		};

		class IconTriangle : public Icon
		{
		public:
			IconTriangle( Drawer *drawer, float size );
			virtual ~IconTriangle();

			virtual bool draw();
		};

		class IconQuad : public Icon
		{
		public:
			IconQuad( Drawer *drawer, float size );
			virtual ~IconQuad();

			virtual bool draw();
		};

		class IconCross : public Icon
		{
		public:
			IconCross( Drawer *drawer, float size );
			virtual ~IconCross();

			virtual bool draw();
		};

		class IconPlus : public Icon
		{
		public:
			IconPlus( Drawer *drawer, float size );
			virtual ~IconPlus();

			virtual bool draw();
		};

		class IconMinus : public Icon
		{
		public:
			IconMinus( Drawer *drawer, float size );
			~IconMinus();

			virtual bool draw();
		};

		class IconArrowTopRight : public Icon
		{
		public:
			IconArrowTopRight( Drawer *drawer, float size );
			virtual ~IconArrowTopRight();

			virtual bool draw();
		};

		class IconArrowLowerLeft : public Icon
		{
		public:
			IconArrowLowerLeft( Drawer *drawer, float size );
			virtual ~IconArrowLowerLeft();

			virtual bool draw();
		};
	}
}
#endif