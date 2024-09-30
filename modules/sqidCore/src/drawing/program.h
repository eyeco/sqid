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

#include <commonGL.h>

#include <map>
#include <vector>

namespace sqid
{
	class Shader;

	class Program
	{
	private:
		struct Uniform
		{
		public:
			Uniform() :
				name( "<unnamed>" ),
				location( -1 )
			{}

			Uniform( const std::string &n, GLint loc ) :
				name( n ),
				location( loc )
			{}

			std::string name;
			GLint location;
		};

	protected:
		std::string _name;

		bool _valid;

		GLuint _id;

		Shader *_vertShader;
		Shader *_fragShader;

		std::map<std::string,GLuint> _attribCache;
		std::map<std::string,Uniform> _uniCache;

		//intentionally no implemented
		//prevent from copying via copy constructor -- have to use reference counting to not end up with invalid gl program IDs
		Program( const Program& );

	public:
		Program( const std::string &name, Shader *vertShader, Shader *fragShader );
		virtual ~Program();

		virtual bool activate();
		virtual bool deactivate();

		virtual bool link();
		virtual void release();

		bool isValid() const	{ return _valid; }

		bool cacheAttribute( const std::string &name );
		GLint getAttribute( const std::string &name, bool cache = true );

		bool cacheUniform( const std::string &name );
		//bool setUniform( const std::string &name, bool value );
		bool setUniform( const std::string &name, const glm::vec4 &v, bool cache = true );
		bool setUniformTex2D( const std::string &name, GLuint tex, int texOffset, bool cache = true );

		const std::string &getName() const { return _name; }
	};
}
#endif