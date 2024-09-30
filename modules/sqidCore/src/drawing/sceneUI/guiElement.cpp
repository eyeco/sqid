/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "guiElement.h"

#ifdef __SUPPORT_GUI

#include "guiDrawer.h"

namespace sqid
{
	namespace GUI
	{
		Element::Element( Drawer *drawer, Anchor anchor ) :
			_clipped( false ),
			_visible( true ),
			_drawer( drawer ),
			_z( 0 ),
			_anchor( anchor ),
			_parent( nullptr )
		{
			_drawer->add( this );
		}

		Element::~Element()
		{
			_drawer->remove( this );
		}

		void Element::preDraw()
		{
			glm::vec2 pxSize( 1.0f / _drawer->getWindowSize().x, 1.0f / _drawer->getWindowSize().y );

			glm::vec2 wPos( getWorldPos() );

			glm::vec2 p0;
			glm::vec2 p1;

			switch( _anchor )
			{
			case A_UPPERLEFT:
				p0 = ( _drawer->getMV() * glm::vec4( wPos.xy, 0, 1 ) ).xy * pxSize;
				p1 = ( _drawer->getMV() * glm::vec4( wPos.xy + _size.xy, 0, 1 ) ).xy * pxSize;
				break;
			case A_CENTER:
				p0 = ( _drawer->getMV() * glm::vec4( wPos.xy - _size.xy * 0.5f, 0, 1 ) ).xy * pxSize;
				p1 = ( _drawer->getMV() * glm::vec4( wPos.xy + _size.xy * 0.5f, 0, 1 ) ).xy * pxSize;
				break;
			case A_LOWERRIGHT:
				p0 = ( _drawer->getMV() * glm::vec4( wPos.xy - _size.xy, 0, 1 ) ).xy * pxSize;
				p1 = ( _drawer->getMV() * glm::vec4( wPos.xy, 0, 1 ) ).xy * pxSize;
				break;
			default:
				std::cerr << "<error> invalid anchor" << std::endl;
			}

			_clipped = ( p0.x > 1 || p0.y > 1 || p1.x < 0 || p1.y < 0 );
		}

		const glm::vec2 Element::getWorldPos() const
		{
			if( _parent )
				return _parent->getWorldPos() + _pos;
			return _pos;
		}

		bool Element::hitTest( const glm::vec2 &worldPos ) const
		{
			glm::vec2 wPos( getWorldPos() );

			switch( _anchor )
			{
			case A_UPPERLEFT:
				return( worldPos.x >= wPos.x && worldPos.y >= wPos.y && worldPos.x < wPos.x + _size.x && worldPos.y < wPos.y + _size.y );
			case A_CENTER:
				return( worldPos.x >= wPos.x - _size.x * 0.5f && worldPos.y >= wPos.y - _size.y * 0.5f && worldPos.x < wPos.x + _size.x * 0.5f && worldPos.y < wPos.y + _size.y * 0.5f );
			case A_LOWERRIGHT:
				return( worldPos.x >= wPos.x - _size.x && worldPos.y >= wPos.y - _size.y && worldPos.x < wPos.x && worldPos.y < wPos.y );
			default:
				std::cerr << "<error> invalid anchor" << std::endl;
			}

			return false;
		}

		bool Element::overlap( const Rect &r ) const
		{
			glm::vec2 wPos( getWorldPos() );

			glm::vec2 p0;
			glm::vec2 p1;

			switch( _anchor )
			{
			case A_UPPERLEFT:
				p0 = wPos;
				p1 = wPos + _size;
				break;
			case A_CENTER:
				p0 = wPos - _size * 0.5f;
				p1 = wPos + _size * 0.5f;
				break;
			case A_LOWERRIGHT:
				p0 = wPos - _size;
				p1 = wPos;
				break;
			default:
				std::cerr << "<error> invalid anchor" << std::endl;
			}

			Rect bb( p0, p1 );

			bool ret = true;
			if( r.P0().x >= bb.P1().x || bb.P0().x >= r.P1().x )
				ret = false;
			if( r.P0().y >= bb.P1().y || bb.P0().y >= r.P1().y )
				ret = false;

			return ret;
		}
	}
}

#endif