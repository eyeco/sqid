/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <drawing/vertexBuffer.h>

#ifdef __SUPPORT_GUI

#include "program.h"

namespace sqid
{
	VertexBuffer::VertexBuffer( const std::string &name ) :
		_name( name ),
		_valid( false ),
		_vboID( ~0x00 ),
		_iboID( ~0x00 ),
		_vertexCount( 0 ),
		_indexCount( 0 ),
		_vertexElementsMask( VE_POSITION | VE_COLOR | VE_UV ),
		_primitiveType( GL_NONE ),
		_indexDataType( GL_NONE ),
		_usage( GL_NONE )
	{}

	VertexBuffer::~VertexBuffer()
	{
		this->destroy();
	}

	bool VertexBuffer::build( const Vertex *vertices, size_t vertexCount, unsigned short *indices, size_t indexCount, GLenum primitiveType, GLenum indexDataType, bool dynamic )
	{
		if( _valid )
			destroy();

		if( ( vertices && !vertexCount ) || !indexCount )
		{
			std::cerr << "<error> count must not be 0" << std::endl;
			return false;
		}

		if( !indices )
		{
			std::cerr << "<error> no indices provided" << std::endl;
			return false;
		}

		_vertexCount = vertexCount;
		_indexCount = indexCount;
		_primitiveType = primitiveType;
		_indexDataType = indexDataType;
		_usage = ( dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW );

		if( !dynamic && !vertices )
		{
			std::cerr << "<error> no vertices specified for static drawing!" << std::endl;
			return true;
		}

		glGenBuffers( 1, &_vboID );
		glBindBuffer( GL_ARRAY_BUFFER, _vboID );
		glBufferData( GL_ARRAY_BUFFER, _vertexCount * sizeof( Vertex ), vertices, _usage );

		glGenBuffers( 1, &_iboID );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _iboID );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, _indexCount * sizeof( unsigned short ), indices, _usage );

		// bind with 0, so, switch back to normal pointer operation
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

		_valid = true;

		return true;
	}

	void VertexBuffer::destroy()
	{
		if( _valid )
		{
			glDeleteBuffers( 1, &_vboID );
			_vboID = ~0x00;

			glDeleteBuffers( 1, &_iboID );
			_iboID = ~0x00;

			_valid = false;
		}
	}

	bool VertexBuffer::activate( Program *program, const Vertex *vertices, size_t vertexCount )
	{
		if( _valid )
		{
			glBindBuffer( GL_ARRAY_BUFFER, _vboID );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _iboID );

			if( vertices )
			{
				if( _usage != GL_DYNAMIC_DRAW )
					std::cerr << "<warning> vertexbuffer is not initialized for dynamic draw, passed vertices will be ignored" << std::endl;
				else
				{
					if( !vertexCount )
						std::cerr << "<error> no vertices provided" << std::endl;
					else
						glBufferSubData( GL_ARRAY_BUFFER, 0, vertexCount * sizeof( Vertex ), vertices );
				}
			}

			int stride = VertexBuffer::getVertexStride();
			int offsetPosition = VertexBuffer::getPositionOffset();
			int offsetColor = VertexBuffer::getColorOffset();
			int offsetUV = VertexBuffer::getUVOffset();

			if( program )
			{
				GLint loc = -1;

				if( _vertexElementsMask & VE_POSITION )
				{
					loc = program->getAttribute( "inPosition" );
					if( loc >= 0 )
					{
						glEnableVertexAttribArray( loc );
						glVertexAttribPointer( loc, 3, GL_FLOAT, GL_FALSE, stride, (void*) ( ( char* )nullptr + offsetPosition ) );
					}
				}

				if( _vertexElementsMask & VE_NORMAL )
				{
					//TODO
					std::cerr << "<error> normals not supported yet" << std::endl;
				}

				if( _vertexElementsMask & VE_COLOR )
				{
					loc = program->getAttribute( "inColor" );
					if( loc >= 0 )
					{
						glEnableVertexAttribArray( loc );
						glVertexAttribPointer( loc, 4, GL_FLOAT, GL_FALSE, stride, (void*) ( ( char* )nullptr + offsetColor ) );
					}
				}

				if( _vertexElementsMask & VE_UV )
				{
					loc = program->getAttribute( "inUV" );
					if( loc >= 0 )
					{
						glEnableVertexAttribArray( loc );
						glVertexAttribPointer( loc, 2, GL_FLOAT, GL_FALSE, stride, (void*) ( ( char* )nullptr + offsetUV ) );
					}
				}
			}
			else
			{
				if( _vertexElementsMask & VE_POSITION )
				{
					glEnableClientState( GL_VERTEX_ARRAY );
					glVertexPointer( 3, GL_FLOAT, stride, (void*) ( ( char* )nullptr + offsetPosition ) );
				}
				if( _vertexElementsMask & VE_NORMAL )
				{
					//TODO
					std::cerr << "<error> normals not supported yet" << std::endl;
				}
				if( _vertexElementsMask & VE_COLOR )
				{
					glEnableClientState( GL_COLOR_ARRAY );
					glColorPointer( 4, GL_FLOAT, stride, (void*) ( ( char* )nullptr + offsetColor ) );
				}
				if( _vertexElementsMask & VE_UV )
				{
					glEnableClientState( GL_TEXTURE_COORD_ARRAY );
					glTexCoordPointer( 2, GL_FLOAT, stride, (void*) ( ( char* )nullptr + offsetUV ) );
				}
			}
		}

		return _valid;
	}

	bool VertexBuffer::deactivate( Program *program )
	{
		if( _valid )
		{
			if( program )
			{
				GLint loc = -1;

				if( _vertexElementsMask & VE_POSITION )
				{
					loc = program->getAttribute( "inPosition" );
					if( loc >= 0 )
						glDisableVertexAttribArray( loc );
				}

				if( _vertexElementsMask & VE_NORMAL )
				{
					//TODO
					std::cerr << "<error> normals not supported yet" << std::endl;
				}

				if( _vertexElementsMask & VE_COLOR )
				{
					loc = program->getAttribute( "inColor" );
					if( loc >= 0 )
						glDisableVertexAttribArray( loc );
				}

				if( _vertexElementsMask & VE_UV )
				{
					loc = program->getAttribute( "inUV" );
					if( loc >= 0 )
						glDisableVertexAttribArray( loc );
				}
			}
			else
			{
				if( _vertexElementsMask & VE_POSITION )
					glDisableClientState( GL_VERTEX_ARRAY );
				if( _vertexElementsMask & VE_NORMAL )
				{
					//TODO
					std::cerr << "<error> normals not supported yet" << std::endl;
				}
				if( _vertexElementsMask & VE_COLOR )
					glDisableClientState( GL_COLOR_ARRAY );
				if( _vertexElementsMask & VE_UV )
					glDisableClientState( GL_TEXTURE_COORD_ARRAY );
			}

			// bind with 0, so, switch back to normal pointer operation
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
		}

		return _valid;
	}

	void VertexBuffer::draw( Program *program )
	{
		draw( program, nullptr, 0 );
	}

	void VertexBuffer::draw( Program *program, const Vertex *vertices, size_t vertexCount )
	{
		if( _valid )
		{
			if( vertices && !vertexCount )
			{
				std::cerr << "<warning> 0 vertices provided" << std::endl;
				return;
			}

			activate( program, vertices, vertexCount );

			glDrawElements( _primitiveType, _indexCount, _indexDataType, nullptr );

			deactivate( program );
		}
	}



	VertexBuffer *VertexBuffer::createAxis( float scale )
	{
		const int vertCount = 6;

		Vertex vertices[vertCount];
		vertices[0] = Vertex( glm::vec3( 0, 0, 0 ), glm::vec4( 1, 0, 0, 1 ), glm::vec2( 0, 0 ) );
		vertices[1] = Vertex( glm::vec3( scale, 0, 0 ), glm::vec4( 1, 0, 0, 1 ), glm::vec2( 0, 0 ) );
		vertices[2] = Vertex( glm::vec3( 0, 0, 0 ), glm::vec4( 0, 1, 0, 1 ), glm::vec2( 0, 0 ) );
		vertices[3] = Vertex( glm::vec3( 0, scale, 0 ), glm::vec4( 0, 1, 0, 1 ), glm::vec2( 0, 0 ) );
		vertices[4] = Vertex( glm::vec3( 0, 0, 0 ), glm::vec4( 0, 0, 1, 1 ), glm::vec2( 0, 0 ) );
		vertices[5] = Vertex( glm::vec3( 0, 0, scale ), glm::vec4( 0, 0, 1, 1 ), glm::vec2( 0, 0 ) );

		unsigned short indices[vertCount];
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "axis" );
		vb->build( vertices, vertCount, indices, vertCount, GL_LINES, GL_UNSIGNED_SHORT, false );

		return vb;
	}

	VertexBuffer *VertexBuffer::createLocator( const glm::vec3 &color, float scale )
	{
		const int vertCount = 6;

		glm::vec4 c( color.rgb, 1 );

		Vertex vertices[vertCount];
		vertices[0] = Vertex( glm::vec3( -scale, 0, 0 ), c, glm::vec2( 0, 0 ) );
		vertices[1] = Vertex( glm::vec3( scale, 0, 0 ), c, glm::vec2( 0, 0 ) );
		vertices[2] = Vertex( glm::vec3( 0, -scale, 0 ), c, glm::vec2( 0, 0 ) );
		vertices[3] = Vertex( glm::vec3( 0, scale, 0 ), c, glm::vec2( 0, 0 ) );
		vertices[4] = Vertex( glm::vec3( 0, 0, -scale ), c, glm::vec2( 0, 0 ) );
		vertices[5] = Vertex( glm::vec3( 0, 0, scale ), c, glm::vec2( 0, 0 ) );

		unsigned short indices[vertCount];
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "locator" );
		vb->build( vertices, vertCount, indices, vertCount, GL_LINES, GL_UNSIGNED_SHORT, false );

		return vb;
	}

	VertexBuffer *VertexBuffer::createRect( const glm::vec2 &size, const glm::vec3 &center )
	{
		const int vertCount = 4;

		Vertex vertices[vertCount];
		vertices[0] = Vertex( center + glm::vec3( -size.x * 0.5f, -size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 0, 1 ) );
		vertices[1] = Vertex( center + glm::vec3( size.x * 0.5f, -size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 1, 1 ) );
		vertices[2] = Vertex( center + glm::vec3( size.x * 0.5f, size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 1, 0 ) );
		vertices[3] = Vertex( center + glm::vec3( -size.x * 0.5f, size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 0, 0 ) );

		unsigned short indices[vertCount];
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "rect" );
		vb->build( vertices, vertCount, indices, vertCount, GL_LINE_LOOP, GL_UNSIGNED_SHORT, false );

		return vb;
	}

	VertexBuffer *VertexBuffer::createQuad( const glm::vec2 &size, const glm::vec3 &center )
	{
		const int vertCount = 4;

		Vertex vertices[vertCount];
		vertices[0] = Vertex( center + glm::vec3( -size.x * 0.5f, -size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 0, 1 ) );
		vertices[1] = Vertex( center + glm::vec3( size.x * 0.5f, -size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 1, 1 ) );
		vertices[2] = Vertex( center + glm::vec3( size.x * 0.5f, size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 1, 0 ) );
		vertices[3] = Vertex( center + glm::vec3( -size.x * 0.5f, size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 0, 0 ) );

		unsigned short indices[vertCount];
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "quad" );
		vb->build( vertices, vertCount, indices, vertCount, GL_QUADS, GL_UNSIGNED_SHORT, false );

		return vb;
	}

	VertexBuffer *VertexBuffer::createGrid( const glm::vec2 &size, unsigned int subdivisions, const glm::vec3 &color )
	{
		const int vertCount = ( subdivisions + 2 ) * 4;

		glm::vec4 c( color.rgb, 1.0f );

		std::vector<Vertex> vertices( vertCount );
		for( int i = 0; i <= subdivisions + 1; i++ )
		{
			float t = i / ( subdivisions + 1.0f ) - 0.5f;

			//hor line
			vertices[i * 4 + 0] = Vertex( glm::vec3( size.x * 0.5f, size.y * t, 0.0f ), c, glm::vec2( 0, 0 ) );
			vertices[i * 4 + 1] = Vertex( glm::vec3( -size.x * 0.5f, size.y * t, 0.0f ), c, glm::vec2( 0, 0 ) );

			//ver line
			vertices[i * 4 + 2] = Vertex( glm::vec3( size.x * t, -size.y * 0.5f, 0.0f ), c, glm::vec2( 0, 0 ) );
			vertices[i * 4 + 3] = Vertex( glm::vec3( size.x * t, size.y * 0.5f, 0.0f ), c, glm::vec2( 0, 0 ) );
		}

		std::vector<unsigned short> indices( vertCount );
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "grid" );
		vb->build( &vertices[0], vertices.size(), &indices[0], indices.size(), GL_LINES, GL_UNSIGNED_SHORT, false );

		return vb;
	}

	VertexBuffer *VertexBuffer::createSphere( float radius, unsigned int subdivsLong, unsigned int subdivsLat, const glm::vec3 &color )
	{
		//taken from http://www.songho.ca/opengl/gl_sphere.html

		unsigned int stackCount = subdivsLat;
		unsigned int sectorCount = subdivsLong;

		//std::vector<float> vertices;
		//std::vector<float> normals;
		//std::vector<float> texCoords;

		std::vector<Vertex> vertices;
		glm::vec4 c( color.rgb, 1.0f );

		float x, y, z, xy;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
		float s, t;                                     // vertex texCoord

		float sectorStep = pi2() / sectorCount;
		float stackStep = pi() / stackCount;
		float sectorAngle, stackAngle;

		for( int i = 0; i <= stackCount; ++i )
		{
			stackAngle = pi() / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xy = radius * cosf( stackAngle );             // r * cos(u)
			z = radius * sinf( stackAngle );              // r * sin(u)

			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for( int j = 0; j <= sectorCount; ++j )
			{
				sectorAngle = j * sectorStep;           // starting from 0 to 2pi

				// vertex position (x, y, z)
				x = xy * cosf( sectorAngle );             // r * cos(u) * cos(v)
				y = xy * sinf( sectorAngle );             // r * cos(u) * sin(v)
				//vertices.push_back( x );
				//vertices.push_back( y );
				//vertices.push_back( z );

				// normalized vertex normal (nx, ny, nz)
				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;
				//normals.push_back( nx );
				//normals.push_back( ny );
				//normals.push_back( nz );

				// vertex tex coord (s, t) range between [0, 1]
				s = (float) j / sectorCount;
				t = (float) i / stackCount;
				//texCoords.push_back( s );
				//texCoords.push_back( t );

				//TODO: add normals
				vertices.push_back( Vertex( glm::vec3( x, y, z ), c, glm::vec2( s, t ) ) );
			}
		}

		// generate CCW index list of sphere triangles
		std::vector<unsigned short> indices;
		int k1, k2;
		for( int i = 0; i < stackCount; ++i )
		{
			k1 = i * ( sectorCount + 1 );     // beginning of current stack
			k2 = k1 + sectorCount + 1;      // beginning of next stack

			for( int j = 0; j < sectorCount; ++j, ++k1, ++k2 )
			{
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if( i != 0 )
				{
					indices.push_back( k1 );
					indices.push_back( k2 );
					indices.push_back( k1 + 1 );
				}

				// k1+1 => k2 => k2+1
				if( i != ( stackCount - 1 ) )
				{
					indices.push_back( k1 + 1 );
					indices.push_back( k2 );
					indices.push_back( k2 + 1 );
				}
			}
		}

#ifdef _DEBUG
		unsigned short maxIndex = 0;
		for( int i = 0; i < indices.size(); i++ )
			maxIndex = max( indices[i], maxIndex );
		if( maxIndex >= vertices.size() )
			std::cerr << "<error> index out of vertex bounds" << std::endl;
#endif

		VertexBuffer *vb = new VertexBuffer( "sphere" );
		vb->build( &vertices[0], vertices.size(), &indices[0], indices.size(), GL_TRIANGLES, GL_UNSIGNED_SHORT, false );

		return vb;
	}
}
#endif