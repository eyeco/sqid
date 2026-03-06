/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "font.h"

#ifdef __SUPPORT_GUI
#include "shader.h"
#include "program.h"

#include <sstream>
#include <iostream>

#ifdef __STATIC_SHADERS
#include "staticShaders.h"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>

#ifdef _WIN32
#pragma comment( lib, "freetype.lib" )
#endif

namespace sqid
{
	Font::Font() :
		vertShader( new VertexShader( "passthrough" ) ),
		fragShader( new FragmentShader( "font" ) ),
		program( nullptr ),
		vertexBuffer( nullptr )
	{
		init();
	};

	Font::Font( const std::string &fontFile, uint32_t fontSize ) :
		vertShader( new VertexShader( "passthrough" ) ),
		fragShader( new FragmentShader( "font" ) ),
		program( nullptr ),
		vertexBuffer( nullptr )
	{
		init();
		load( fontFile, fontSize );
	}

	Font::~Font()
	{
		safeDelete( program );
		safeDelete( fragShader );
		safeDelete( vertShader );

		safeDelete( vertexBuffer );
	}

	void Font::init()
	{
		unsigned short indices[] = { 0, 1, 2, 3 };

		vertices.resize( 4 );

		vertices[0].uv = glm::vec2( 0, 0 );
		vertices[1].uv = glm::vec2( 1, 0 );
		vertices[2].uv = glm::vec2( 1, 1 );
		vertices[3].uv = glm::vec2( 0, 1 );

#ifdef __STATIC_SHADERS
		vertShader->compileSource( vertSourcePassthrough );
		fragShader->compileSource( fragSourceFont );
#else
		vertShader->compileFromFile( "resources/shaders/passthrough.vert" );
		fragShader->compileFromFile( "resources/shaders/font.frag" );
#endif

		program = new Program( "font", vertShader, fragShader );
		program->link();

		program->cacheUniform( "tex" );

		vertexBuffer = new VertexBuffer( "font" );
		vertexBuffer->build( &vertices[0], 4, indices, 4, GL_QUADS, GL_UNSIGNED_SHORT, true );
	}

	bool Font::load( const std::string &fontFile, uint32_t fontSize )
	{
		characters.clear();

		FT_Library ft;
		if( FT_Init_FreeType( &ft ) )
		{
			std::cerr << "<error> init freetype2 failed" << std::endl;
			return false;
		}

		FT_Face face;
		if( FT_New_Face( ft, fontFile.c_str(), 0, &face ) )
		{
			std::cerr << "<error> failed to load font from file \"" << fontFile << "\"" << std::endl;

			FT_Done_FreeType( ft );
			return false;
		}

		// Set size to load glyphs as
		FT_Set_Pixel_Sizes( face, 0, fontSize );

		// Disable byte-alignment restriction
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		// Then for the first 128 ASCII characters, pre-load/compile their characters and store them
		for( GLubyte c = 0; c < 128; c++ ) // lol see what I did there 
		{
			// Load character glyph 
			if( FT_Load_Char( face, c, FT_LOAD_RENDER ) )
			{
				std::cout << "<error> failed to load freetype glyph #" << (int)c << std::endl;

				Character character = {
					(uint)~0x00,
					glm::ivec2( 0, 0 ),
					glm::ivec2( 0, 0 ),
					0
				};

				characters.push_back( character );

				continue;
			}

			// Generate texture
			GLuint texture;
			glGenTextures( 1, &texture );
			glBindTexture( GL_TEXTURE_2D, texture );
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// Set texture options
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			// Now store character for later use
			Character character = {
				texture,
				glm::ivec2( face->glyph->bitmap.width, face->glyph->bitmap.rows ),
				glm::ivec2( face->glyph->bitmap_left, face->glyph->bitmap_top ),
				(uint32_t)face->glyph->advance.x
			};

			characters.push_back( character );
		}
		glBindTexture( GL_TEXTURE_2D, 0 );

		// Destroy FreeType once we're finished
		FT_Done_Face( face );

		FT_Done_FreeType( ft );

		return true;
	}

	void Font::print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight )
	{
		print( text, x, y, canvasWidth, canvasHeight, 1.0f );
	}

	void Font::print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, const glm::vec3 &color )
	{
		print( text, x, y, canvasWidth, canvasHeight, 1.0f, color );
	}

	void Font::print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, const glm::vec4 &color )
	{
		print( text, x, y, canvasWidth, canvasHeight, 1.0f, color );
	}

	void Font::print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, float scale )
	{
		print( text, x, y, canvasWidth, canvasHeight, scale, glm::vec4( 1 ) );
	}

	void Font::print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, float scale, const glm::vec3 &color )
	{
		print( text, x, y, canvasWidth, canvasHeight, scale, glm::vec4( color.x, color.y, color.z, 1.0f ) );
	}

	void Font::print( const std::string &text, GLfloat x, GLfloat y, unsigned int canvasWidth, unsigned int canvasHeight, GLfloat scale, const glm::vec4 &color )
	{
		if( characters.empty() || text.empty() )
			return;

		if( !program || !vertexBuffer )
			return;

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glDisable( GL_DEPTH_TEST );
			glDisable( GL_CULL_FACE );
			glDisable( GL_LIGHTING );

			glEnable( GL_COLOR_MATERIAL );
			glEnable( GL_TEXTURE_2D );
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			vertices[0].col = color;
			vertices[1].col = color;
			vertices[2].col = color;
			vertices[3].col = color;

			program->activate();

			// Iterate through all characters
			std::string::const_iterator c;
			for( c = text.begin(); c != text.end(); c++ )
			{
				Character ch = characters[*c];

				GLfloat x0 = x + ch.bearing.x * scale;
				GLfloat y0 = y + ( characters['H'].bearing.y - ch.bearing.y ) * scale;

				GLfloat x1 = x0 + ch.size.x * scale;
				GLfloat y1 = y0 + ch.size.y * scale;
				
				x0 = ( x0 / canvasWidth - 0.5f ) * 2.0f;
				y0 = -( y0 / canvasHeight - 0.5f ) * 2.0f;

				x1 = ( x1 / canvasWidth - 0.5f ) * 2.0f;
				y1 = -( y1 / canvasHeight - 0.5f ) * 2.0f;
				
				vertices[0].p = glm::vec3( x0, y0, 0 );
				vertices[1].p = glm::vec3( x1, y0, 0 );
				vertices[2].p = glm::vec3( x1, y1, 0 );
				vertices[3].p = glm::vec3( x0, y1, 0 );

				// Render glyph texture over quad
				glBindTexture( GL_TEXTURE_2D, ch.textureID );

				program->setUniformTex2D( "tex", ch.textureID, 0 );

				vertexBuffer->draw( program, &vertices[0], vertices.size() );

				// Now advance cursors for next glyph
				x += ( ch.advance >> 6 ) * scale; // Bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
			}

			program->deactivate();
		}
		glPopAttrib();
	}
}
#endif