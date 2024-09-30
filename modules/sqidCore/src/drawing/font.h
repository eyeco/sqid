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
#include <glm/glm.hpp>

#include <vector>

#include <drawing/vertexBuffer.h>

namespace sqid
{
	class Shader;
	class Program;

	class Font
	{
	private:
		struct Character
		{
			uint32_t textureID;
			glm::ivec2 size;
			glm::ivec2 bearing;
			uint32_t advance;
		};

		std::vector<Character> characters;

		std::vector<VertexBuffer::Vertex> vertices;

		Shader *vertShader;
		Shader *fragShader;
		Program *program;
		VertexBuffer *vertexBuffer;

		void init();

	public:
		Font();
		Font( const std::string &fontFile, uint32_t fontSize );
		~Font();

		bool load( const std::string &fontFile, uint32_t fontSize );

		void print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight );
		void print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, float scale );
		void print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, const glm::vec3 &color );
		void print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, const glm::vec4 &color );
		void print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, float scale, const glm::vec3 &color );
		void print( const std::string &text, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, float scale, const glm::vec4 &color );
	};
}
#endif