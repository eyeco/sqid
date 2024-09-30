/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "commonGL.h"

namespace sqid
{
#ifdef __SUPPORT_GUI
#ifdef _DEBUG
	SQID_API bool SQID_API_CALL checkForGLError()
	{
		GLenum errCode;
		if( ( errCode = glGetError() ) != GL_NO_ERROR )
		{
			const GLubyte *str = gluErrorString( errCode );
			if( str )
				std::cerr << "<error> OpenGL error: " << str << std::endl;
			else
				std::cerr << "<error> unknown OpenGL error #" << std::hex << errCode << std::dec << std::endl;
			return false;
		}
		return true;
	}
#else
	SQID_API bool SQID_API_CALL checkForGLError() { return true; }
#endif //_DEBUG


	SQID_API void SQID_API_CALL drawQuad( float left, float bottom, float right, float top )
	{
		std::vector<GLfloat> vertices( 8 );

		vertices[0] = left;
		vertices[1] = bottom;

		vertices[2] = right;
		vertices[3] = bottom;

		vertices[4] = right;
		vertices[5] = top;

		vertices[6] = left;
		vertices[7] = top;

		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 2, GL_FLOAT, 0, &vertices[0] );

		glDrawArrays( GL_TRIANGLE_FAN, 0, 8 );

		// deactivate vertex arrays after drawing
		glDisableClientState( GL_VERTEX_ARRAY );
	}

	SQID_API void SQID_API_CALL drawGraph( const std::vector<float> &values, unsigned int maxValues )//, float bottom, float top )
	{
		if( values.size() )
		{
			std::vector<GLfloat> vertices( values.size() * 2 );

			for( int i = 0; i < values.size(); i++ )
			{
				vertices[i * 2 + 0] = (float) i / ( maxValues - 1.0f );
				vertices[i * 2 + 1] = values[i];
			}

			glEnableClientState( GL_VERTEX_ARRAY );
			glVertexPointer( 2, GL_FLOAT, 0, &vertices[0] );

			glDrawArrays( GL_LINE_STRIP, 0, (GLsizei) values.size() );
			//glDrawArrays( GL_POINTS, 0, values.size() );

			// deactivate vertex arrays after drawing
			glDisableClientState( GL_VERTEX_ARRAY );
		}
	}

	SQID_API void SQID_API_CALL drawAxes()
	{
		static glm::vec3 verts[] = {
			zero(), unitX(),
			zero(), unitY(),
			zero(), unitZ()
		};

		static glm::vec3 cols[] = {
			red(), red(),
			green(), green(),
			blue(), blue()
		};

		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_COLOR_ARRAY );

		glVertexPointer( 3, GL_FLOAT, 0, verts );
		glColorPointer( 3, GL_FLOAT, 0, cols );

		glDrawArrays( GL_LINES, 0, arraySize( verts ) );

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
	}

	SQID_API void SQID_API_CALL drawLocator()
	{
		//TODO
	}

	SQID_API void SQID_API_CALL drawGrid()
	{
		static glm::vec3 verts[] = {
			glm::vec3( -5, 0, -5 ), glm::vec3( 5, 0, -5 ),
			glm::vec3( -5, 0, -4 ), glm::vec3( 5, 0, -4 ),
			glm::vec3( -5, 0, -3 ), glm::vec3( 5, 0, -3 ),
			glm::vec3( -5, 0, -2 ), glm::vec3( 5, 0, -2 ),
			glm::vec3( -5, 0, -1 ), glm::vec3( 5, 0, -1 ),
			glm::vec3( -5, 0, 0 ), glm::vec3( 5, 0, 0 ),
			glm::vec3( -5, 0, 1 ), glm::vec3( 5, 0, 1 ),
			glm::vec3( -5, 0, 2 ), glm::vec3( 5, 0, 2 ),
			glm::vec3( -5, 0, 3 ), glm::vec3( 5, 0, 3 ),
			glm::vec3( -5, 0, 4 ), glm::vec3( 5, 0, 4 ),
			glm::vec3( -5, 0, 5 ), glm::vec3( 5, 0, 5 ),

			glm::vec3( -5, 0, -5 ), glm::vec3( -5, 0, 5 ),
			glm::vec3( -4, 0, -5 ), glm::vec3( -4, 0, 5 ),
			glm::vec3( -3, 0, -5 ), glm::vec3( -3, 0, 5 ),
			glm::vec3( -2, 0, -5 ), glm::vec3( -2, 0, 5 ),
			glm::vec3( -1, 0, -5 ), glm::vec3( -1, 0, 5 ),
			glm::vec3( 0, 0, -5 ), glm::vec3( 0, 0, 5 ),
			glm::vec3( 1, 0, -5 ), glm::vec3( 1, 0, 5 ),
			glm::vec3( 2, 0, -5 ), glm::vec3( 2, 0, 5 ),
			glm::vec3( 3, 0, -5 ), glm::vec3( 3, 0, 5 ),
			glm::vec3( 4, 0, -5 ), glm::vec3( 4, 0, 5 ),
			glm::vec3( 5, 0, -5 ), glm::vec3( 5, 0, 5 )
		};

		glEnableClientState( GL_VERTEX_ARRAY );

		glVertexPointer( 3, GL_FLOAT, 0, verts );

		glPushAttrib( GL_CURRENT_BIT );
		{
			glColor3fv( glm::value_ptr( white() ) );
			glDrawArrays( GL_LINES, 0, arraySize( verts ) );
		}
		glPopAttrib();

		glDisableClientState( GL_VERTEX_ARRAY );
	}
#endif
}