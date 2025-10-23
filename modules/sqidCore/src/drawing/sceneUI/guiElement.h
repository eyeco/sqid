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

#include <vector>

#include <glm/glm.hpp>

namespace sqid
{
	namespace GUI
	{
		class Drawer;

		class Hoverable
		{
		private:
			bool _hovered;

		public:
			Hoverable() :
				_hovered( false )
			{}

			virtual ~Hoverable()
			{}

			bool getHovered() const { return _hovered; }
			void setHovered( bool hover ) { _hovered = hover; }
		};

		class Selectable
		{
		private:
			bool _selected;

		public:
			Selectable() :
				_selected( false )
			{}

			virtual ~Selectable()
			{}

			bool getSelected() const { return _selected; }
			void setSelected( bool select ) { _selected = select; }
		};

		class Pressable
		{
		private:
			bool _pressed;

		public:
			Pressable() :
				_pressed( false )
			{}

			virtual ~Pressable()
			{}

			bool getPressed() const { return _pressed; }
			void setPressed( bool pressed ) { _pressed = pressed; }
		};

		class Draggable
		{
		private:
			bool _dragging;

		public:
			Draggable() :
				_dragging( false )
			{}

			virtual ~Draggable()
			{}

			bool getDragging() const { return _dragging; }
			void setDragging( bool dragging ) { _dragging = dragging; }
		};

		class Element : 
			public MouseEventHandler,
			public KeyEventHandler
		{
		public:
			enum Anchor
			{
				A_UPPERLEFT,
				A_CENTER,
				A_LOWERRIGHT,

				A_COUNT
			};

		protected:
			bool _clipped;
			bool _visible;

			Drawer *_drawer;

			glm::vec2 _pos;
			glm::vec2 _size;

			float _z;

			Anchor _anchor;

			Element *_parent;

		public:
			explicit Element( Drawer *drawer, Anchor anchor = A_UPPERLEFT );
			virtual ~Element();

			virtual void preDraw();
			virtual bool draw()		{ return _visible && !_clipped; }
			virtual void postDraw()	{}

			virtual bool drawUI()	{ return false; }

			float getZ() const { return _z; }
			virtual float setZ( float z ) { _z = z; return z; }

			const glm::vec2 &getPos() const { return _pos; }
			const glm::vec2 getWorldPos() const;

			const glm::vec2 &getSize() const { return _size; }

			bool getVisible() const { return _visible; }
			void setVisible( bool visible ) { _visible = visible; }

			virtual void setPos( const glm::vec2 &pos ) { _pos = pos; }

			virtual bool hitTest( const glm::vec2 &worldPos ) const;
			virtual bool overlap( const Rect &r ) const;

			Element *getParent() const { return _parent; }
			void setParent( Element *parent ) { _parent = parent; }
		};
	}
}

#endif