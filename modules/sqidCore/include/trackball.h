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

#include <common.h>

#include <glm/glm.hpp>

namespace sqid
{
	class SQID_API Trackball :
		public MouseEventHandler,
		public KeyEventHandler
	{
	protected:
		bool _firstPos;
		bool _firstScroll;

		glm::vec2 _pos;
		glm::vec2 _oldPos;

		glm::vec2 _scroll;

		unsigned int _buttonMask;

		glm::mat4 _m;

		virtual void update() = 0;

		virtual bool onMouseMotion( const glm::vec2 &pos, const glm::vec2 &d ) { return false; }
		virtual bool onMouseDown( int button ) { return false; }
		virtual bool onMouseUp( int button ) { return false; }
		virtual bool onScroll( const glm::vec2 &scroll, const glm::vec2 &d ) { return false; }

		virtual bool onCharDown( unsigned char c );
		virtual bool onKeyDown( int key, int scanCode, int action, int mods ) { return false; }

	public:
		Trackball();
		~Trackball();

		virtual void reset() = 0;

		virtual bool mouseDown( int button, int mods, bool imGuiHandled );
		virtual bool mouseUp( int button, int mods, bool imGuiHandled );
		virtual void mouseMotion( const glm::vec2 &pos );

		virtual bool scroll( const glm::vec2 &offset, bool imGuiHandled );

		virtual bool keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled ) { return onKeyDown( key, scanCode, action, mods ); }
		virtual bool charDown( unsigned char c, bool imGuiHandled ) { return onCharDown( c ); }

		const glm::mat4 &mat() const { return _m; }
	};

	class SQID_API Trackball2D : public Trackball
	{
	private:
		bool _relativeMode;

		glm::vec3 _trans;
		float _motionSpeed;

		float _rotation;
		float _rotationSpeed;

		float _scale;
		float _scaleSpeed;
		float _wheelSpeed;
		float _scalePow;

		glm::vec3 _defaultTrans;
		float _defaultRotation;
		float _defaultScale;
		
		virtual void update();

		virtual bool onMouseMotion( const glm::vec2 &pos, const glm::vec2 &d );
		virtual bool onScroll( const glm::vec2 &scroll, const glm::vec2 &d );

	public:
		Trackball2D();
		Trackball2D( const glm::vec3 &t, float r, float s );
		~Trackball2D();

		virtual void reset();
	};

	class SQID_API Trackball3D : public Trackball
	{
	private:
		bool _relativeMode;

		glm::vec3 _trans;

		float _motionSpeed;
		float _wheelSpeed;

		glm::vec3 _euler;
		float _rotationSpeed;

		glm::vec3 _defaultTrans;
		glm::vec3 _defaultEuler;

		virtual void update();

		virtual bool onMouseMotion( const glm::vec2 &pos, const glm::vec2 &d );
		virtual bool onScroll( const glm::vec2 &scroll, const glm::vec2 &d );

	public:
		Trackball3D();
		Trackball3D( const glm::vec3 &t, const glm::vec3 &e );
		~Trackball3D();

		virtual void reset();
	};
}
#endif