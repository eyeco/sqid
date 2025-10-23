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

#include <commonGL.h>

#include <glm/glm.hpp>

namespace sqid
{
	class Program;

	class SQID_API VertexBuffer
	{
	public:
		enum VertexElements
		{
			VE_NONE,

			VE_POSITION	= 0x01 << 0,
			VE_NORMAL	= 0x01 << 1,
			VE_COLOR	= 0x01 << 2,
			VE_UV		= 0x01 << 3
		};

		struct Vertex
		{
			glm::vec3 p;
			glm::vec4 col;
			glm::vec2 uv;

			Vertex()
			{}

			Vertex( const glm::vec3 &p, const glm::vec4 &col, const glm::vec2 &uv ) :
				p( p ), col( col ), uv( uv )
			{}
		};

	private:
		std::string _name;

		bool _valid;

		GLuint _vboID;
		GLuint _iboID;

		size_t _vertexCount;
		size_t _indexCount;
		unsigned int _vertexElementsMask;

		GLenum _primitiveType;
		GLenum _indexDataType;
		GLenum _usage;

		bool activate( Program *program = nullptr, const Vertex *vertices = nullptr, size_t vertexCount = 0 );
		bool deactivate( Program *program = nullptr );

		//intentionally no implemented
		//prevent from copying via copy constructor -- have to use reference counting to not end up with invalid gl VBO IDs
		VertexBuffer( const VertexBuffer& );

	public:
		explicit VertexBuffer( const std::string &name );
		~VertexBuffer();

		bool build( const Vertex *vertices, size_t vertexCount, unsigned short *indices, size_t indexCount, GLenum primitiveType, GLenum indexDataType, bool dynamic );
		void destroy();

		void draw( Program *program = nullptr );
		void draw( Program *program, const Vertex *vertices, size_t vertexCount );

		static inline int getVertexStride()		{	return sizeof( Vertex );	}
		static inline int getPositionOffset()	{	return offsetof( Vertex, p );	}
		static inline int getColorOffset()		{	return offsetof( Vertex, col );	}
		static inline int getUVOffset()			{	return offsetof( Vertex, uv );	}

		static VertexBuffer *createAxis( float scale = 1.0f );
		static VertexBuffer *createLocator( const glm::vec3 &color = glm::vec3( 0.5f ), float scale = 1.0f );

		static VertexBuffer *createRect( const glm::vec2 &size = glm::vec2( 1 ), const glm::vec3 &center = glm::vec3( 0 ) );
		static VertexBuffer *createQuad( const glm::vec2 &size = glm::vec2( 1 ), const glm::vec3 &center = glm::vec3( 0 ) );
		static VertexBuffer *createGrid( const glm::vec2 &size = glm::vec2( 1 ), unsigned int subdivisions = 9, const glm::vec3 &color = glm::vec3( 0.75f ) );

		static VertexBuffer *createSphere( float radius = 1.0f, unsigned int subdivsLong = 10, unsigned int subdivsLat = 10, const glm::vec3 &color = glm::vec3( 1 ) );
	};
}
#endif