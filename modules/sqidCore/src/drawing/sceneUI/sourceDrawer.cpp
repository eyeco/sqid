/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "sourceDrawer.h"

#ifdef __SUPPORT_GUI

#include <fileIO/json.h>

#include <sources/dataSource.h>

#include <commonImGui.h>

#include "../font.h"
#include "guiDrawer.h"
#include "uiStyle.h"

namespace sqid
{
	SourceDrawer::SourceDrawer( GUI::Drawer *drawer, DataSource *source, const glm::vec2 &pos ) :
		NodeDrawer( drawer ),
		_source( source )
	{
		_pos = pos;
	}

	SourceDrawer::~SourceDrawer()
	{
		_source = nullptr;
	}

	void SourceDrawer::build()
	{
		_verts.resize( 4 );
		_verts[0] = glm::vec2( 0, 0 );
		_verts[1] = glm::vec2( 0, _size.y );
		_verts[2] = glm::vec2( _size.x, _size.y );
		_verts[3] = glm::vec2( _size.x, 0 );

		_vertsSelection.resize( 4 );
		_vertsSelection[0] = _verts[0] + glm::vec2( -_drawer->getUIStyle()->NodeSelectionLineDist, -_drawer->getUIStyle()->NodeSelectionLineDist );
		_vertsSelection[1] = _verts[1] + glm::vec2( -_drawer->getUIStyle()->NodeSelectionLineDist, _drawer->getUIStyle()->NodeSelectionLineDist );
		_vertsSelection[2] = _verts[2] + glm::vec2( _drawer->getUIStyle()->NodeSelectionLineDist, _drawer->getUIStyle()->NodeSelectionLineDist );
		_vertsSelection[3] = _verts[3] + glm::vec2( _drawer->getUIStyle()->NodeSelectionLineDist, -_drawer->getUIStyle()->NodeSelectionLineDist );
	}


	bool SourceDrawer::draw()
	{
		if( !NodeDrawer::draw() )
			return false;

		glPushMatrix();
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glTranslatef( _pos.x, _pos.y, 0.0f );

			glm::vec3 cols[4];

			if( getSelected() )
			{
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeSelectionLineColor;

				glLineWidth( _drawer->getUIStyle()->NodeSelectionLineWidth );

				glColorPointer( 3, GL_FLOAT, 0, cols );
				glVertexPointer( 2, GL_FLOAT, 0, &_vertsSelection[0] );
				glDrawArrays( GL_LINE_LOOP, 0, _vertsSelection.size() );
			}

			glLineWidth( _drawer->getUIStyle()->NodeLineWidth );

			glVertexPointer( 2, GL_FLOAT, 0, &_verts[0] );

			if( getDragging() )
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeColorDraggingBG;
			else if( getHovered() )
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeColorHoveredBG;
			else
				cols[0] = cols[1] = cols[2] = cols[3] = _drawer->getUIStyle()->NodeColorBG;

			glColorPointer( 3, GL_FLOAT, 0, cols );

			glDrawArrays( GL_QUADS, 0, _verts.size() );
		}
		glPopAttrib();
		glPopMatrix();

		glm::vec4 textPos( _drawer->getMV() * glm::vec4( _pos.x + _drawer->getUIStyle()->NodePaddingX, _pos.y + _drawer->getUIStyle()->TextDY, 0, 1 ) );
		_drawer->getUIStyle()->getFont().print(
			formatInterfaceString( _source->getDeviceInterface(), _source->getDevicePort() ),
			textPos.x, textPos.y, _drawer->getWindowSize().x, _drawer->getWindowSize().y, _drawer->getMV()[2][2], _drawer->getUIStyle()->TextColor );

		return true;
	}

	bool SourceDrawer::drawUI()
	{
		bool ret = NodeDrawer::drawUI();

		//push ID so equally-named elements don't override cross-node when switching between nodes of same type
		ImGui::PushID( guidToString( _source->getObjectID() ).c_str() );

		if( ret && _source )
			_source->drawUI();

		ImGui::PopID();

		return ret;
	}

	bool SourceDrawer::saveToJSON( nlohmann::json &j ) const
	{
		bool ret = NodeDrawer::saveToJSON( j );

		if( _source )
			save( j, "objectID", _source->getObjectID() );

		return ret;
	}

	bool SourceDrawer::loadFromJSON( const nlohmann::json &j )
	{
		bool ret = NodeDrawer::loadFromJSON( j );

		return ret;
	}
}
#endif