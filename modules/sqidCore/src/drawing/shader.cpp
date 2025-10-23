/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "shader.h"

#ifdef __SUPPORT_GUI

namespace sqid
{
	Shader::Shader( const std::string &name, GLenum type ) :
		_name( name ),
		_valid( false ),
		_id( ~0x00 ),
		_type( type )
	{}

	Shader::~Shader()
	{
		this->release();
	}

	bool Shader::compileSource( const char *source )
	{
		this->release();

		_id = glCreateShader( _type );
		_valid = true;

		GLint compiled = GL_FALSE;

		if( !glIsShader( _id ) )
			std::cerr << "shader invalid" << std::endl;
		else
		{
			const char *sourcePtr = &source[0];
			glShaderSource( _id, 1, &sourcePtr, 0 );
			glCompileShader( _id );
			glGetShaderiv( _id, GL_COMPILE_STATUS, &compiled );
			if( compiled == GL_FALSE )
			{
				GLint maxLength = 0;
				glGetShaderiv( _id, GL_INFO_LOG_LENGTH, &maxLength );

				std::vector<GLchar> infoLog( maxLength );
				glGetShaderInfoLog( _id, maxLength, &maxLength, &infoLog[0] );

				std::cerr << "<error> compiling shader \"" << _name << "\" failed: " << &infoLog[0] << std::endl;
			}
		}

		if( !compiled )
			this->release();

		return compiled;
	}

	bool Shader::compileSource( const std::string &source )
	{
		this->release();

		if( !source.size() )
		{
			std::cerr << "<error> shader source code string is empty" << std::endl;
			return false;
		}

		return compileSource( source.c_str() );
	}

	bool Shader::compileFromFile( const std::string &path )
	{
		this->release();

		FILE *fp = fopen( path.c_str(), "r" );
		if( !fp )
		{
			std::cerr << "<error> failed to open shader file: " << path << std::endl;
			return false;
		}

		fseek( fp, 0, SEEK_END );
		size_t fileSize = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		if( fileSize < 0 )
		{
			std::cerr << "<error> failed getting file size of file: " << path << std::endl;
			return false;
		}

		std::vector<char> source( fileSize );
		int read = 0;
		if( ( read = fread( &source[0], 1, source.size(), fp ) ) != source.size() )
			std::cerr << "<warning> did not read expected number of bytes (" << read << " vs. " << source.size() << ")" << std::endl;

		fclose( fp );

		if( !compileSource( &source[0] ) )
		{
			std::cerr << "<error> compiling shader from file " << path << " failed" << std::endl;
			return false;
		}

		return true;
	}

	void Shader::release()
	{
		if( _valid )
		{
			glDeleteShader( _id );
			_id = ~0x00;
			_valid = false;
		}
	}


	VertexShader::VertexShader( const std::string &name ) :
		Shader( name, GL_VERTEX_SHADER )
	{}

	VertexShader::~VertexShader()
	{}



	FragmentShader::FragmentShader( const std::string &name ) :
		Shader( name, GL_FRAGMENT_SHADER )
	{}

	FragmentShader::~FragmentShader()
	{}
}
#endif