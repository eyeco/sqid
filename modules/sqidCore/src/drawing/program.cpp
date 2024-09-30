/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "program.h"

#ifdef __SUPPORT_GUI

#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

namespace sqid
{
	Program::Program( const std::string &name, Shader *vertShader, Shader *fragShader ) :
		_name( name ),
		_valid( false ),
		_id( ~0x00 ),
		_vertShader( vertShader ),
		_fragShader( fragShader )
	{}

	Program::~Program()
	{
		this->release();

		_valid = false;

		_vertShader = nullptr;
		_fragShader = nullptr;
	}

	bool Program::activate()
	{
		if( !_valid )
			return false;

		glUseProgram( _id );

		return true;
	}

	bool Program::deactivate()
	{
		glUseProgram( 0 );

		return true;
	}

	void Program::release()
	{
		if( _valid )
		{
			glDeleteProgram( _id );
			_id = ~0x00;
			_valid = false;
		}

		_attribCache.clear();
		_uniCache.clear();
	}

	bool Program::link()
	{
		this->release();

		if( !_vertShader || !_fragShader )
		{
			std::cerr << "shaders must not be null" << std::endl;
			return false;
		}

		if( _vertShader->_id == ~0x00 || _fragShader->_id == ~0x00 )
		{
			std::cerr << "shader invalid or not compiled yet" << std::endl;
			return false;
		}

		_id = glCreateProgram();
		_valid = true;

		GLint linked = GL_FALSE;

		glAttachShader( _id, _vertShader->_id );
		glAttachShader( _id, _fragShader->_id );
		glLinkProgram( _id );

		glGetProgramiv( _id, GL_LINK_STATUS, &linked );
		if( linked == GL_FALSE )
		{
			GLint maxLength = 0;
			glGetProgramiv( _id, GL_INFO_LOG_LENGTH, &maxLength );

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog( maxLength );
			glGetProgramInfoLog( _id, maxLength, &maxLength, &infoLog[0] );

			std::cerr << "<error> linking program: " << &infoLog[0] << std::endl;
		}

		// Always detach shaders after a successful link.
		glDetachShader( _id, _vertShader->_id );
		glDetachShader( _id, _fragShader->_id );

		if( !linked )
			this->release();

		return linked;
	}

	bool Program::cacheAttribute( const std::string &name )
	{
		GLint loc = glGetAttribLocation( _id, name.c_str() );
		_attribCache.insert( std::make_pair( name, loc ) );

		return ( loc != -1 );
	}

	GLint Program::getAttribute( const std::string &name, bool cache )
	{
		GLint loc = -1;

		if( cache )
		{
			auto it = _attribCache.find( name );
			if( it == _attribCache.end() )
			{
				loc = glGetAttribLocation( _id, name.c_str() );
				_attribCache.insert( std::make_pair( name, loc ) );
			}
			else
				loc = it->second;
		}
		else
			loc = glGetAttribLocation( _id, name.c_str() );

		return loc;
	}

	bool Program::cacheUniform( const std::string &name )
	{
		GLint loc = glGetUniformLocation( _id, name.c_str() );
		_uniCache.insert( std::make_pair( name, Uniform( name, loc ) ) );

		return true;
	}

	bool Program::setUniform( const std::string &name, const glm::vec4 &v, bool cache )
	{
		GLint loc = -1;

		if( cache )
		{
			auto it = _uniCache.find( name );
			if( it == _uniCache.end() )
			{
				loc = glGetUniformLocation( _id, name.c_str() );
				_uniCache.insert( std::make_pair( name, Uniform( name, loc ) ) );
			}
			else
				loc = it->second.location;
		}
		else
			loc = glGetUniformLocation( _id, name.c_str() );

		glUniform4fv( loc, 1, glm::value_ptr( v ) );

		return true;
	}

	bool Program::setUniformTex2D( const std::string &name, GLuint tex, int texOffset, bool cache )
	{
		GLint loc = -1;

		if( cache )
		{
			auto it = _uniCache.find( name );
			if( it == _uniCache.end() )
			{
				loc = glGetUniformLocation( _id, name.c_str() );
				_uniCache.insert( std::make_pair( name, Uniform( name, loc ) ) );
			}
			else
				loc = it->second.location;
		}
		else
			loc = glGetUniformLocation( _id, name.c_str() );

		glUniform1i( loc, texOffset );
		glActiveTexture( GL_TEXTURE0 + texOffset );
		glBindTexture( GL_TEXTURE_2D, tex );

		return true;
	}
}

#endif