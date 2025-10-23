/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "frameDrawerGeneral.h"

#ifdef __SUPPORT_GUI

#include <processing/op.h>

#include "../processing/ops/general.h"

#include <sampleFrame.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sqid
{
	DEFINE_FRAMEDRAWER_DESC( ContourDrawer, "contours", "668115C5-303C-4A6E-A2E9-D03E0FEF8BCD" );

	ContourDrawer::ContourDrawer( const General::ContourDetector *detector ) :
		FrameDrawer2D(),
		_detector( detector )
	{
	}

	ContourDrawer::~ContourDrawer()
	{
		_detector = nullptr;
	}

	void ContourDrawer::lateDraw( const SampleFrame *sf )
	{
		FrameDrawer2D::lateDraw( sf );

		if( !_detector )
			return;

		if( !sf )
			return;

		auto contours = _detector->getContours();

		float w = sf->width();
		float h = sf->height();
		float invA = 1.0f / w * h;
		float sx = 2.0f / w;
		float sy = 2.0f / h;

		int biggestContourIndex = -1;
		float biggestContourArea = 0;
		for( int i = 0; i < contours.size(); i++ )
			if( contours[i].area > biggestContourArea )
			{
				biggestContourArea = contours[i].area;
				biggestContourIndex = i;
			}

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glEnable( GL_COLOR_MATERIAL );
			glDisable( GL_TEXTURE_2D );

			glLineWidth( 2.0f );

			for( int i = 0; i < contours.size(); i++ )
			{
				auto &c = contours[i];

				if( i == biggestContourIndex )
					glColor3f( 0, 1, 1 );
				else
					glColor3f( 0, 0, 1 );

				glBegin( GL_LINE_LOOP );
				{
					for( auto &p : c.points )
						glVertex2f( ( p.x + 0.5f ) * sx - 1, -( ( p.y + 0.5f ) * sy - 1 ) );
				}
				glEnd();

				float cx = ( c.centroid.x + 0.5f ) * sx - 1;
				float cy = -( ( c.centroid.y + 0.5f ) * sy - 1 );
				float crossSize = 0.05f;
				glBegin( GL_LINES );
				{
					glVertex2f( cx - crossSize, cy - crossSize );
					glVertex2f( cx + crossSize, cy + crossSize );

					glVertex2f( cx - crossSize, cy + crossSize );
					glVertex2f( cx + crossSize, cy - crossSize );
				}
				glEnd();

				glColor3f( 0, 0, 0 );

				char tempStr[128];
				sprintf( tempStr, "A: %.02f", c.area * invA );

				printText( tempStr,
					c.centroid.x / getWidth() * getRTSize(), c.centroid.y / getHeight() * getRTSize(),
					getRTSize(), getRTSize(),
					glm::vec4( 1 ), 3.0f );
			}
		}
		glPopAttrib();
	}
}
#endif