/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <trackball.h>

#ifdef __SUPPORT_GUI
#include <glm/gtx/transform.hpp>


namespace sqid
{
	Trackball::Trackball() :
		_firstPos( true ),
		_firstScroll( true ),
		_pos( 0 ),
		_oldPos( 0 ),
		_scroll( 0 ),
		_buttonMask( 0 ),
		_m( 1 )
	{}

	Trackball::~Trackball()
	{}

	bool Trackball::mouseDown( int button, int mods, bool imGuiHandled )
	{
		_buttonMask |= ( 0x01 << button );

		return onMouseDown( button );
	}

	bool Trackball::mouseUp( int button, int mods, bool imGuiHandled )
	{
		_buttonMask &= ~( 0x01 << button );

		return onMouseUp( button );
	}

	void Trackball::mouseMotion( const glm::vec2 &pos )
	{
		glm::vec2 d = ( _firstPos ? glm::vec2( 0 ) : pos - _oldPos );

		_oldPos = _pos;
		_pos = pos;

		onMouseMotion( pos, d );

		_firstPos = false;
	}

	bool Trackball::scroll( const glm::vec2 &offset, bool imGuiHandled )
	{
		_scroll += offset;

		return onScroll( _scroll, offset );
	}

	bool Trackball::onCharDown( unsigned char c )
	{
		switch( c )
		{
		case 'r':
			reset();
			return true;
		}

		return false;
	}




	Trackball2D::Trackball2D() :
		Trackball(),
		_relativeMode( false ),
		_trans( 0 ),
		_motionSpeed( 0.001f ),
		_rotation( 0 ),
		_rotationSpeed( 0.01f ),
		_scale( 0 ),
		_scaleSpeed( 0.01f ),
		_wheelSpeed( 0.1f ),
		_scalePow( 2.0f ),
		_defaultTrans( 0 ),
		_defaultRotation( 0 ),
		_defaultScale( 0 )
	{}

	Trackball2D::Trackball2D( const glm::vec3 &t, float r, float s ) :
		Trackball(),
		_relativeMode( false ),
		_trans( t ),
		_motionSpeed( 0.001f ),
		_rotation( r ),
		_rotationSpeed( 0.01f ),
		_scale( s ),
		_scaleSpeed( 0.01f ),
		_wheelSpeed( 0.1f ),
		_scalePow( 2.0f ),
		_defaultTrans( t ),
		_defaultRotation( r ),
		_defaultScale( s )
	{
		reset();
	}

	Trackball2D::~Trackball2D()
	{}

	void Trackball2D::reset()
	{
		_m = glm::translate( _defaultTrans )
			* glm::rotate( _defaultRotation, unitZ() )
			* glm::scale( glm::vec3( max( 0.000001f, pow( _scalePow, _defaultScale ) ) ) );
	}

	bool Trackball2D::onMouseMotion( const glm::vec2 &pos, const glm::vec2 &d )
	{
		if( _buttonMask & ( 0x01 << 0 ) ) //left
		{
			if( _relativeMode )
				_m = glm::rotate( -d.x * _rotationSpeed, unitZ() ) * _m;
			else
				_rotation += -d.x * _rotationSpeed;
		}
		if( _buttonMask & ( 0x01 << 1 ) ) //middle
		{
			if( _relativeMode )
				_m = glm::scale( glm::vec3( max( 0.000001f, pow( _scalePow, -d.y * _scaleSpeed ) ) ) ) * _m;
			else
				_scale += -d.y * _scaleSpeed;
		}
		if( _buttonMask & ( 0x01 << 2 ) ) //right
		{
			if( _relativeMode )
				_m = glm::translate( glm::vec3( d.x, -d.y, 0 ) * _motionSpeed ) * _m;
			else
				_trans += glm::vec3( d.x, -d.y, 0 ) * _motionSpeed;
		}

		if( _buttonMask )
			update();

		return true;
	}

	bool Trackball2D::onScroll( const glm::vec2 &scroll, const glm::vec2 &d )
	{
		if( _relativeMode )
			_m = glm::scale( glm::vec3( max<float>( 0.000001f, pow( _scalePow, d.y * _wheelSpeed ) ) ) ) * _m;
		else
			_scale += d.y * _wheelSpeed;

		update();
		
		onMouseMotion( _pos, glm::vec2( 0 ) );

		return true;
	}

	void Trackball2D::update()
	{
		if( !_relativeMode )
		{
			_m = glm::translate( _trans )
				* glm::rotate( _rotation, unitZ() )
				* glm::scale( glm::vec3( max( 0.000001f, pow( _scalePow, _scale ) ) ) )
				;
		}
	}




	Trackball3D::Trackball3D() :
		Trackball(),
		_relativeMode( false ),
		_trans( 0 ),
		_motionSpeed( 0.001f ),
		_wheelSpeed( 0.5f ),
		_euler( 0 ),
		_rotationSpeed( 0.01f ),
		_defaultTrans( 0 ),
		_defaultEuler( 0 )
	{}

	Trackball3D::Trackball3D( const glm::vec3 &t, const glm::vec3 &e ) :
		Trackball(),
		_relativeMode( false ),
		_trans( t ),
		_motionSpeed( 0.001f ),
		_wheelSpeed( 0.5f ),
		_euler( e ),
		_rotationSpeed( 0.01f ),
		_defaultTrans( t ),
		_defaultEuler( e )
	{
		reset();
	}

	Trackball3D::~Trackball3D()
	{}

	void Trackball3D::reset()
	{
		_m = glm::translate( _defaultTrans )
			* glm::rotate( _defaultEuler.x, unitX() )
			* glm::rotate( _defaultEuler.y, unitY() )
			* glm::rotate( _defaultEuler.z, unitZ() );
	}

	bool Trackball3D::onMouseMotion( const glm::vec2 &pos, const glm::vec2 &d )
	{
		if( _buttonMask & ( 0x01 << 0 ) ) //left
		{
			if( _relativeMode )
				_m = glm::rotate( d.x * _rotationSpeed, unitY() ) * glm::rotate( d.y * _rotationSpeed, unitX() ) * _m;
			else
				_euler += glm::vec3( d.y, d.x, 0 ) * _rotationSpeed;
		}
		if( _buttonMask & ( 0x01 << 2 ) ) //middle
		{
			if( _relativeMode )
				_m = glm::translate( glm::vec3( 0, 0, -d.y ) * _motionSpeed ) * _m;
			else
				_trans += glm::vec3( 0, 0, -d.y ) * _motionSpeed;
		}
		if( _buttonMask & ( 0x01 << 1 ) ) //right
		{
			if( _relativeMode )
				_m = glm::translate( glm::vec3( d.x, -d.y, 0 ) * _motionSpeed ) * _m;
			else
				_trans += glm::vec3( d.x, -d.y, 0 ) * _motionSpeed;
		}

		if( _buttonMask )
			update();

		return true;
	}

	bool Trackball3D::onScroll( const glm::vec2 &scroll, const glm::vec2 &d )
	{
		if( _relativeMode )
			_m = glm::translate( glm::vec3( 0, 0, d.y ) * _wheelSpeed ) * _m;
		else
			_trans += glm::vec3( 0, 0, d.y ) * _wheelSpeed;
		update();

		onMouseMotion( _pos, glm::vec2( 0 ) );

		return true;
	}

	void Trackball3D::update()
	{
		if( !_relativeMode )
		{
			_m = glm::translate( _trans )
				* glm::rotate( _euler.x, unitX() )
				* glm::rotate( _euler.y, unitY() )
				* glm::rotate( _euler.z, unitZ() )
				;
		}
	}
}
#endif