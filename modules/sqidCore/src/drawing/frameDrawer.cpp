/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <drawing/frameDrawer.h>

#ifdef __SUPPORT_GUI

#include "shader.h"
#include "program.h"

#include <processing/op.h>
#include <drawing/frameBuffer.h>
#include <drawing/vertexBuffer.h>

#ifdef __STATIC_SHADERS
#include "staticShaders.h"
#endif

#include <commonImGui.h>

#include <app.h>
#include <trackball.h>

#include <sampleFrame.h>

#include <fileIO/json.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace sqid
{
	DEFINE_FRAMEDRAWER_DESC( FrameDrawerLines, "lines (x=t)", "A2BDB410-F257-4360-8615-B821BE548FBD" );
	DEFINE_FRAMEDRAWER_DESC( HistoryDrawerLines, "lines (hist)", "DD1010AE-A63C-4736-906A-E7FF4C8DCAEB" );
	DEFINE_FRAMEDRAWER_DESC( HistoryDrawerMap, "map (hist)", "723FA6EE-CB10-4ED5-848C-1F75672ED1F1" );
	DEFINE_FRAMEDRAWER_DESC( FrameDrawer2D, "map", "BFC43774-23B5-4692-82C3-00A09EC72114" );
	DEFINE_FRAMEDRAWER_DESC( FrameDrawer3D, "3D", "97E6BA70-17B8-4EE6-9E27-66AEFDC8CC21" );
	DEFINE_FRAMEDRAWER_DESC( PointCloudDrawer, "pcl", "1DB8B997-4EC7-460C-A5DD-FC4CEB89A312" );

	namespace Internal
	{
		static VertexBuffer *quad = nullptr;

		static Shader *vertShader = nullptr;
		static Shader *fragShaderSC = nullptr;
		static Shader *fragShaderMC = nullptr;

		static Program *programSC = nullptr;
		static Program *programMC = nullptr;

		static glm::vec4 clearColor( 29 / 255.0f, 47 / 255.0f, 74 / 255.0f, 1.0f );
		static glm::vec4 infoColor( 147 / 255.0f, 235 / 255.0f, 255 / 255.0f, 1.0f );
		static glm::vec4 inspectionColor( 255 / 255.0f, 255 / 255.0f, 0 / 255.0f, 1.0f );
	}

	using namespace Internal;

	bool FrameDrawer::init()
	{
		bool ret = true;

		quad = VertexBuffer::createQuad( glm::vec2( 2, 2 ) ); // range is [-1 1]
		ret &= ( quad != nullptr );

		vertShader = new VertexShader( "passthrough" );
		fragShaderSC = new FragmentShader( "singleChannel" );
		fragShaderMC = new FragmentShader( "multiChannel" );

#ifdef __STATIC_SHADERS
		ret &= vertShader->compileSource( vertSourcePassthrough );
		ret &= fragShaderSC->compileSource( fragSourceSingleChannel );
		ret &= fragShaderMC->compileSource( fragSourceMultiChannel );
#else
		ret &= vertShader->compileFromFile( "resources/shaders/passthrough.vert" );
		ret &= fragShaderSC->compileFromFile( "resources/shaders/singleChannel.frag" );
		ret &= fragShaderMC->compileFromFile( "resources/shaders/multiChannel.frag" );
#endif

		programSC = new Program( "singleChannel", vertShader, fragShaderSC );
		programMC = new Program( "multiChannel", vertShader, fragShaderMC );

		ret &= programSC->link();
		ret &= programMC->link();

		ret &= programSC->cacheAttribute( "inPosition" );
		ret &= programSC->cacheAttribute( "inColor" );
		ret &= programSC->cacheAttribute( "inUV" );

		ret &= programSC->cacheUniform( "tex" );
		ret &= programMC->cacheUniform( "tex" );

		return ret;
	}

	bool FrameDrawer::uninit()
	{
		safeDelete( programSC );
		safeDelete( programMC );
		safeDelete( vertShader );
		safeDelete( fragShaderSC );
		safeDelete( fragShaderMC );

		safeDelete( quad );

		return true;
	}



	FrameDrawer::FrameDrawer( size_t rtSize, bool flipRB ) :
		_flipRB( flipRB ),
		_rtSize( rtSize ),
		_fb( new FrameBuffer( "frameDrawer" ) ),
		_sf( nullptr )
	{
		if( !_fb->build( rtSize, rtSize ) )
			std::cerr << "<error> failed to build framebuffer" << std::endl;
	}

	FrameDrawer::~FrameDrawer()
	{
		safeDelete( _fb );
		safeDelete( _sf );
	}

	GLuint FrameDrawer::getRTName() const
	{
		return _fb->getRenderTextureName();
	}

	bool FrameDrawer::update( const SampleFrame *sf )
	{
		safeDelete( _sf );
		_sf = new SampleFrame( *sf );

		return ( sf != nullptr );
	}

	void FrameDrawer::draw()
	{
		draw( _fb, _rtSize, _rtSize );
	}

	bool FrameDrawer::drawUI()
	{
		return true;
	}

	bool FrameDrawer::loadFromJSON( const nlohmann::json &j )
	{
#ifdef _DEBUG
		std::string name;
		load<std::string>( j, "name", name );
		if( strcmp( getName(), name.c_str() ) )
			std::cerr << "<warning> name does not match (" << getName() << ": " << name << " should be " << getName() << ")" << std::endl;
		std::string classID;
		load<std::string>( j, "classID", classID );
		if( getClassID() != guidFromString( classID ) )
			std::cerr << "<warning> ClassID does not match (" << getName() << ": " << classID << " should be " << guidToString( getClassID() ) << ")" << std::endl;
#endif

		return true;
	}

	bool FrameDrawer::saveToJSON( nlohmann::json &j ) const
	{
		save( j, "classID", getClassID() );
		save( j, "name", getName() );

		return true;
	}





	FrameDrawerLines::FrameDrawerLines( float lineWidth ) :
		FrameDrawer( 200 ),
		_autoScale( false ),
		_scale( 1.0f ),
		_offset( 0.0f ),
		_drawInfo( true ),
		_lineWidth( lineWidth ),
		_vertices( 0 ),
		_isSelected( false ),
		_selectionVertices( 4 )
	{
		_selectionVertices[1] = -1;
		_selectionVertices[3] = 1;
		//_selectionVertices[4] = -1;
		//_selectionVertices[6] = 1;
	}

	FrameDrawerLines::~FrameDrawerLines()
	{
	}

	void FrameDrawerLines::checkVertices( size_t size )
	{
		if( _vertices.size() != size * 2 )
		{
			_vertices.resize( size * 2 );
			for( int i = 0; i < size; i++ )
				_vertices[i * 2] = ( (float) i / ( size - 1.0f ) ) * 2.0f - 1.0f;
		}
	}

	void FrameDrawerLines::draw( FrameBuffer *fb, size_t width, size_t height )
	{
		if( !_sf )
			return;

		int vertexCount = ( _sf->width() == 1 ? 2 : _sf->width() );

		checkVertices( vertexCount );

		if( fb )
			fb->activate();

		int lineCount = _sf->height() * _sf->depth();
		int stride = _sf->width();

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glViewport( 0, 0, width, height );

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			glClearColor( clearColor.r, clearColor.g, clearColor.b, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			if( vertexCount )
			{
				glDisable( GL_DEPTH_TEST );
				glDisable( GL_CULL_FACE );
				glDisable( GL_LIGHTING );
				glDisable( GL_TEXTURE_2D );

				glEnable( GL_COLOR_MATERIAL );

				glEnableClientState( GL_VERTEX_ARRAY );

				float maxValue = 1.0f;
				float minValue = 0.0f;

				float offset = _offset;
				float scale = _scale;

				if( ( _drawInfo || _autoScale ) && _sf->size() )
				{
					const float *vals = _sf->values();

					minValue = vals[0];
					maxValue = vals[0];

					size_t size = _sf->size();

					for( int i = 0; i < size; i++ )
					{
						minValue = min( vals[i], minValue );
						maxValue = max( vals[i], maxValue );
					}

					if( _autoScale )
					{
						float d = maxValue - minValue;
						if( abs( d ) > 0.000001f )
							scale = 2.0f / d;

						offset = -( maxValue + minValue ) * 0.5f * scale;
					}
				}

				if( _drawInfo )
				{
					glLineWidth( min( 0.5f, _lineWidth / 2.0f ) );

					float verts[4] = {
						-1, 0, 1, 0
					};

					verts[1] = verts[3] = offset;

					glColor4fv( glm::value_ptr( infoColor ) );
					glVertexPointer( 2, GL_FLOAT, 0, verts );
					glDrawArrays( GL_LINES, 0, 2 );

					glLineWidth( min( 0.5f, _lineWidth / 4.0f ) );

					glLineStipple( 4, 0xAAAA );
					glEnable( GL_LINE_STIPPLE );

					verts[1] = verts[3] = minValue * scale + offset;
					glDrawArrays( GL_LINES, 0, 2 );

					verts[1] = verts[3] = maxValue * scale + offset;
					glDrawArrays( GL_LINES, 0, 2 );

					verts[1] = verts[3] = ( minValue + maxValue ) * 0.5f * scale + offset;
					glLineStipple( 4, 0xE4E4 );
					glDrawArrays( GL_LINES, 0, 2 );

					glDisable( GL_LINE_STIPPLE );
				}

				glLineWidth( _lineWidth );

				glm::vec3 c0( 61 / 255.0f, 113 / 255.0f, 224 / 255.0f );
				glm::vec3 c1( 1, 0, 0 );

				const float *vals = _sf->values();
				for( int j = 0; j < lineCount; j++, vals += stride )
				{
					if( _sf->width() == 1 )
					{
						_vertices[1] = vals[0] * scale + offset;
						_vertices[3] = _vertices[1];
					}
					else
						for( int i = 0; i < vertexCount; i++ )
							_vertices[i * 2 + 1] = vals[i] * scale + offset;

					if( lineCount > 1 )
						glColor3fv( glm::value_ptr( c0 + ( c1 - c0 ) * ( j / ( lineCount - 1.0f ) ) ) );
					else
						glColor3fv( glm::value_ptr( c1 ) );

					glVertexPointer( 2, GL_FLOAT, 0, &_vertices[0] );
					glDrawArrays( GL_LINE_STRIP, 0, (GLsizei) vertexCount );
				}

				int snappedIndex = 0;
				if( _isSelected && lineCount )
				{
					glPointSize( 10.0f );
					glLineWidth( _lineWidth );

					glm::vec2 snappedPos = _selectionPos;

					if( vertexCount > 1 )
					{
						snappedIndex = clamp<float>( roundf( ( _selectionPos.x + 1.0f ) * 0.5f * ( vertexCount - 1 ) ), 0.0f, vertexCount - 1 );
						snappedPos.x = snappedIndex / ( vertexCount - 1.0f ) * 2.0f - 1.0f;
					}
					else
						snappedPos.x = 0;

					_selectionVertices[0] = _selectionVertices[2] = snappedPos.x;

					glColor4fv( glm::value_ptr( inspectionColor ) );

					glVertexPointer( 2, GL_FLOAT, 0, &_selectionVertices[0] );
					glDrawArrays( GL_LINES, 0, (GLsizei) _selectionVertices.size() / 2 );
					
					const float *vals = _sf->values();

					_inspectedVertices.resize( 2 * lineCount );
					for( int i = 0; i < lineCount; i++ )
					{
						_inspectedVertices[i * 2 + 0] = snappedPos.x;
						_inspectedVertices[i * 2 + 1] = *( vals + stride * i + snappedIndex ) * scale + offset;
					}

					glVertexPointer( 2, GL_FLOAT, 0, &_inspectedVertices[0] );
					glDrawArrays( GL_POINTS, 0, (GLsizei) _inspectedVertices.size() / 2 );
				}

				// deactivate vertex arrays after drawing
				glDisableClientState( GL_VERTEX_ARRAY );

				if( _isSelected && lineCount )
				{
					char tempStr[128];
					const float *vals = _sf->values();
					for( int i = 0; i < lineCount; i++ )
					{
						sprintf( tempStr, "%.3f", *( vals + stride * i + snappedIndex ) );
						printText( tempStr,
							( _inspectedVertices[i * 2 + 0] + 1.0f ) * 0.5f * getRTSize(), 
							( 1.0f - _inspectedVertices[i * 2 + 1] ) * 0.5f * getRTSize(),
							getRTSize(), getRTSize(),
							inspectionColor, 1.0f );
					}
				}

				if( _drawInfo )
				{
					char tempStr[128];

					sprintf( tempStr, "max: %.3f", maxValue );
					printText( tempStr,
						0, ( ( 1.0f - ( maxValue * scale + offset ) ) * 0.5f ) * getRTSize(),
						getRTSize(), getRTSize(),
						infoColor, 1.0f );

					sprintf( tempStr, "min: %.3f", minValue );
					printText( tempStr,
						0, ( ( 1.0f - ( minValue * scale + offset ) ) * 0.5f ) * getRTSize(),
						getRTSize(), getRTSize(),
						infoColor, 1.0f );
				}
			}
		}
		glPopAttrib();

		if( fb )
			fb->deactivate();
	}

	bool FrameDrawerLines::drawUI()
	{
		bool ret = FrameDrawer::drawUI();

		ImGui::Text( getName() );

		ImGui::SliderFloat( "line strength", &_lineWidth, 0.1f, 5.0f );
		ImGui::Checkbox( "auto scale", &_autoScale );

		{
			ScopedImGuiDisable disable( _autoScale );

			ImGui::SliderFloat( "offset", &_offset, -1, 1 );
			ImGui::SliderFloat( "scale", &_scale, 0, 5 );
		}

		ImGui::Checkbox( "draw info", &_drawInfo );

		return ret;
	}

	bool FrameDrawerLines::loadFromJSON( const nlohmann::json &j )
	{
		bool ret = FrameDrawer::loadFromJSON( j );

		load<float>( j, "lineWidth", _lineWidth );

		load<bool>( j, "autoScale", _autoScale );
		load<float>( j, "offset", _offset );
		load<float>( j, "scale", _scale );

		load<bool>( j, "drawInfo", _drawInfo );

		return ret;
	}

	bool FrameDrawerLines::saveToJSON( nlohmann::json &j ) const
	{
		bool ret = FrameDrawer::saveToJSON( j );

		save( j, "lineWidth", _lineWidth );

		save( j, "autoScale", _autoScale );
		save( j, "offset", _offset );
		save( j, "scale", _scale );

		save( j, "drawInfo", _drawInfo );

		return ret;
	}

	bool FrameDrawerLines::mouseDown( int button, int mods, bool imGuiHandled )
	{
		FrameDrawer::mouseDown( button, mods, imGuiHandled );

		if( imGuiHandled )
			return false;

		if( button == 0 )
		{
			_isSelected = true;
			_isMouseDown = true;
		}

		return true;
	}

	bool FrameDrawerLines::mouseUp( int button, int mods, bool imGuiHandled )
	{
		FrameDrawer::mouseUp( button, mods, imGuiHandled );

		if( imGuiHandled )
			return false;

		if( button == 0 )
			_isMouseDown = false;
		if( button == 1 )
			_isSelected = false;

		return true;
	}

	void FrameDrawerLines::mouseMotion( const glm::vec2 &pos )
	{
		FrameDrawer::mouseMotion( pos );

		if( _isSelected && _isMouseDown )
		{
			_selectionPos.x = pos.x / App().getWindowSize().x * 2.0f - 1.0f;
			_selectionPos.y = pos.y / App().getWindowSize().y * 2.0f - 1.0f;
		}
	}

	bool FrameDrawerLines::scroll( const glm::vec2 &offset, bool imGuiHandled )
	{
		return FrameDrawer::scroll( offset, imGuiHandled );
	}

	bool FrameDrawerLines::charDown( unsigned char c, bool imGuiHandled )
	{
		return FrameDrawer::charDown( c, imGuiHandled );
	}

	bool FrameDrawerLines::keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled )
	{
		return FrameDrawer::keyDown( key, scanCode, action, mods, imGuiHandled );
	}





	HistoryDrawerLines::HistoryDrawerLines( size_t bufferSize, float lineWidth ) :
		FrameDrawerLines( lineWidth ),
		_bufferSize( bufferSize )
	{}

	HistoryDrawerLines::~HistoryDrawerLines()
	{
		clear();
	}

	void HistoryDrawerLines::clear()
	{
		for( auto sf : _sfs )
			safeDelete( sf );
		_sfs.clear();
	}

	bool HistoryDrawerLines::update( const SampleFrame *sf )
	{
		if( !_bufferSize )
			return false;

		size_t size = sf->size();
		if( _sfs.size() && _sfs.front()->size() != size )
		{
			std::cout << "size changed, have to clear history" << std::endl;
			clear();
		}

		if( sf )
			_sfs.push_back( new SampleFrame( *sf ) );

		while( _sfs.size() > _bufferSize )
		{
			safeDelete( _sfs.front() );
			_sfs.pop_front();
		}

		return ( sf != nullptr );
	}

	void HistoryDrawerLines::checkVertices( size_t size )
	{
		if( _vertices.size() != size * 2 )
		{
			_vertices.resize( size * 2 );
			for( int i = 0; i < size; i++ )
				_vertices[i * 2] = ( (float) i / ( size - 1.0f ) ) * 2.0f - 1.0f;
		}
	}

	void HistoryDrawerLines::draw( FrameBuffer *fb, size_t width, size_t height )
	{
		if( !_sfs.size() )
			return;

		size_t size = _sfs.front()->size();
		checkVertices( _sfs.size() );

		if( fb )
			fb->activate();

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glViewport( 0, 0, width, height );

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			glClearColor( 29 / 255.0f, 47 / 255.0f, 74 / 255.0f, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			glDisable( GL_DEPTH_TEST );
			glDisable( GL_CULL_FACE );
			glDisable( GL_LIGHTING );
			glDisable( GL_TEXTURE_2D );

			glEnable( GL_COLOR_MATERIAL );

			glEnableClientState( GL_VERTEX_ARRAY );

			float maxValue = 1.0f;
			float minValue = 0.0f;

			float offset = _offset;
			float scale = _scale;

			if( ( _drawInfo || _autoScale ) && ( _sfs.size() && _sfs.front()->size() ) )
			{
				const float *vals = _sfs.front()->values();

				minValue = vals[0];
				maxValue = vals[0];

				for( auto &it : _sfs )
				{
					vals = it->values();
					size_t size = it->size();
					for( int i = 0; i < size; i++ )
					{
						minValue = min( vals[i], minValue );
						maxValue = max( vals[i], maxValue );
					}
				}

				if( _autoScale )
				{
					float d = maxValue - minValue;
					if( abs( d ) > 0.000001f )
						scale = 2.0f / d;

					offset = -( maxValue + minValue ) * 0.5f * scale;
				}
			}

			if( _drawInfo )
			{
				glLineWidth( min( 0.5f, _lineWidth / 2.0f ) );

				float verts[4] = {
					-1, 0, 1, 0
				};

				verts[1] = verts[3] = offset;

				glColor4fv( glm::value_ptr( infoColor ) );
				glVertexPointer( 2, GL_FLOAT, 0, verts );
				glDrawArrays( GL_LINES, 0, 2 );

				glLineWidth( min( 0.5f, _lineWidth / 4.0f ) );

				glLineStipple( 4, 0xAAAA );
				glEnable( GL_LINE_STIPPLE );

				verts[1] = verts[3] = minValue * scale + offset;
				glDrawArrays( GL_LINES, 0, 2 );

				verts[1] = verts[3] = maxValue * scale + offset;
				glDrawArrays( GL_LINES, 0, 2 );

				verts[1] = verts[3] = ( minValue + maxValue ) * 0.5f * scale + offset;
				glLineStipple( 4, 0xE4E4 );
				glDrawArrays( GL_LINES, 0, 2 );

				glDisable( GL_LINE_STIPPLE );
			}

			glLineWidth( _lineWidth );

			glm::vec3 c0( 61 / 255.0f, 113 / 255.0f, 224 / 255.0f );
			glm::vec3 c1( 1, 0, 0 );

			for( int j = 0; j < size; j++ )
			{
				auto it = _sfs.begin();
				for( int i = 0; i < _sfs.size(); i++, ++it )
					_vertices[i * 2 + 1] = ( *it )->values()[j] * scale + offset;

				if( size > 1 )
					glColor3fv( glm::value_ptr( c0 + ( c1 - c0 ) * ( j / ( size - 1.0f ) ) ) );
				else
					glColor3fv( glm::value_ptr( c1 ) );

				glVertexPointer( 2, GL_FLOAT, 0, &_vertices[0] );
				glDrawArrays( GL_LINE_STRIP, 0, (GLsizei) _sfs.size() );
			}

			int snappedIndex = 0;
			if( _isSelected && _sfs.size() )
			{
				glPointSize( 10.0f );
				glLineWidth( _lineWidth );

				glm::vec2 snappedPos = _selectionPos;

				if( _sfs.size() > 1 )
				{
					snappedIndex = clamp<float>( roundf( ( _selectionPos.x + 1.0f ) * 0.5f * ( _sfs.size() - 1 ) ), 0, _sfs.size() - 1 );
					snappedPos.x = snappedIndex / ( _sfs.size() - 1.0f ) * 2.0f - 1.0f;
				}
				else
					snappedPos.x = 0;

				_selectionVertices[0] = _selectionVertices[2] = snappedPos.x;

				glColor4fv( glm::value_ptr( inspectionColor ) );

				glVertexPointer( 2, GL_FLOAT, 0, &_selectionVertices[0] );
				glDrawArrays( GL_LINES, 0, (GLsizei) _selectionVertices.size() / 2 );

				auto it = _sfs.begin();
				for( int i = 0; i < snappedIndex; i++ )
					it++;
				const float *vals = ( *it )->values();

				_inspectedVertices.resize( 2 * size );
				for( int i = 0; i < size; i++ )
				{
					_inspectedVertices[i * 2 + 0] = snappedPos.x;
					_inspectedVertices[i * 2 + 1] = vals[i] * scale + offset;
				}

				glVertexPointer( 2, GL_FLOAT, 0, &_inspectedVertices[0] );
				glDrawArrays( GL_POINTS, 0, (GLsizei) _inspectedVertices.size() / 2 );
			}

			// deactivate vertex arrays after drawing
			glDisableClientState( GL_VERTEX_ARRAY );

			if( _isSelected && _sfs.size() )
			{
				auto it = _sfs.begin();
				for( int i = 0; i < snappedIndex; i++ )
					it++;
				const float *vals = ( *it )->values();

				char tempStr[128];
				for( int i = 0; i < size; i++ )
				{
					sprintf( tempStr, "%.3f", vals[i] );
					printText( tempStr,
						( _inspectedVertices[i * 2 + 0] + 1.0f ) * 0.5f * getRTSize(),
						( 1.0f - _inspectedVertices[i * 2 + 1] ) * 0.5f * getRTSize(),
						getRTSize(), getRTSize(),
						inspectionColor, 1.0f );
				}
			}

			if( _drawInfo )
			{
				char tempStr[128];

				sprintf( tempStr, "max: %.3f", maxValue );
				printText( tempStr,
					0, ( ( 1.0f - ( maxValue * scale + offset ) ) * 0.5f ) * getRTSize(),
					getRTSize(), getRTSize(),
					infoColor, 1.0f );

				sprintf( tempStr, "min: %.3f", minValue );
				printText( tempStr,
					0, ( ( 1.0f - ( minValue * scale + offset ) ) * 0.5f ) * getRTSize(),
					getRTSize(), getRTSize(),
					infoColor, 1.0f );
			}
		}
		glPopAttrib();

		if( fb )
			fb->deactivate();
	}

	bool HistoryDrawerLines::drawUI()
	{
		bool ret = FrameDrawerLines::drawUI();

		int s = _bufferSize;
		if( ImGui::SliderInt( "buffer size", &s, 1, 512 ) )
			_bufferSize = s;

		return ret;
	}

	bool HistoryDrawerLines::loadFromJSON( const nlohmann::json &j )
	{
		bool ret = FrameDrawerLines::loadFromJSON( j );

		load<size_t>( j, "bufferSize", _bufferSize );

		return ret;
	}

	bool HistoryDrawerLines::saveToJSON( nlohmann::json &j ) const
	{
		bool ret = FrameDrawerLines::saveToJSON( j );

		save( j, "bufferSize", _bufferSize );

		return ret;
	}




	FrameDrawer2D::FrameDrawer2D() :
		FrameDrawer( 512 ),
		_texWidth( 0 ),
		_texHeight( 0 ),
		_texDepth( 0 ),
		_texFormat( ~0x00 ),
		_texType( GL_FLOAT ),
		_texName( ~0x00 )
	{}

	FrameDrawer2D::~FrameDrawer2D()
	{
		releaseTexture();
	}

	bool FrameDrawer2D::update( const SampleFrame *sf )
	{
		if( !FrameDrawer::update( sf ) )
			return false;

		if( !checkTexture( sf ) )
			return false;

		return true;
	}

	void FrameDrawer2D::draw( FrameBuffer *fb, size_t width, size_t height )
	{
		if( !_sf )
			return;

		glBindTexture( GL_TEXTURE_2D, _texName );
		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, _texWidth, _texHeight, _texFormat, _texType, _sf->values() );
		glBindTexture( GL_TEXTURE_2D, 0 );

		if( fb )
			fb->activate();

		Program *program = ( _sf->depth() == 1 ? programSC : programMC );

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glViewport( 0, 0, width, height );

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			glClearColor( 1, 0, 1, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			glDisable( GL_DEPTH_TEST );
			glDisable( GL_CULL_FACE );
			glDisable( GL_LIGHTING );

			glEnable( GL_COLOR_MATERIAL );
			glEnable( GL_TEXTURE_2D );

			program->activate();
			program->setUniformTex2D( "tex", _texName, 0 );

			quad->draw( program );

			program->deactivate();

			lateDraw( _sf );
		}
		glPopAttrib();

		if( fb )
			fb->deactivate();
	}

	void FrameDrawer2D::lateDraw( const SampleFrame *sf )
	{}

	bool FrameDrawer2D::checkTexture( size_t width, size_t height, size_t depth )
	{
		if( _texName != ~0x00 && ( _texWidth != width || _texHeight != height || _texDepth != depth ) )
			releaseTexture();

		if( _texName == ~0x00 )
		{
			_texWidth = width;
			_texHeight = height;
			_texDepth = depth;

			if( _texWidth * _texHeight * _texDepth )
			{
				glGenTextures( 1, &_texName );

				glBindTexture( GL_TEXTURE_2D, _texName );

				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

				switch( _texDepth )
				{
				case 1:
					_texFormat = GL_RED;
					glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, _texWidth, _texHeight, 0, _texFormat, _texType, NULL );
					break;
				case 2:
					_texFormat = GL_RG;
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RG32F, _texWidth, _texHeight, 0, _texFormat, _texType, NULL );
					break;
					break;
				case 3:
					_texFormat = ( _flipRB ? GL_BGR : GL_RGB );
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, _texWidth, _texHeight, 0, _texFormat, _texType, NULL );
					break;
				case 4:
					_texFormat = ( _flipRB ? GL_BGRA : GL_RGBA );
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, _texWidth, _texHeight, 0, _texFormat, _texType, NULL );
					break;
				default:
					std::cerr << "<error> texture depth of " << _texDepth << " not supported" << std::endl;
					return false;
				}
			}
			else
			{
#ifdef _DEBUG
				std::cerr << "<warning> texture size is 0" << std::endl;
#endif
				return false;
			}
		}

		return ( _texName != ~0x00 );
	}

	bool FrameDrawer2D::checkTexture( const SampleFrame *sf )
	{
		if( !sf )
			return false;
		return checkTexture( sf->width(), sf->height(), sf->depth() );
	}

	void FrameDrawer2D::releaseTexture()
	{
		if( _texName != ~0x00 )
		{
			glDeleteTextures( 1, &_texName );

			_texName = ~0x00;
			_texWidth = 0;
			_texHeight = 0;
			_texDepth = 0;
		}
	}





	FrameDrawer3D::FrameDrawer3D() :
		FrameDrawer( 512 ),
		_proj( glm::perspectiveFov( (float) toRad( 60.0f ), 1.0f, 1.0f, 0.1f, 100.0f ) ),
		_trackball( new Trackball3D( glm::vec3( 0, 0, -5 ), glm::vec3( toRad( 20.0f ), toRad( -30.0f ), toRad( 0.0f ) ) ) )
	{}

	FrameDrawer3D::~FrameDrawer3D()
	{
		safeDelete( _trackball );
	}

	bool FrameDrawer3D::update( const SampleFrame *sf )
	{
		return FrameDrawer::update( sf );
	}

	void FrameDrawer3D::draw( FrameBuffer *fb, size_t width, size_t height )
	{
		if( fb )
			fb->activate();

		//Program *program = ( sf->depth() == 1 ? programSC : programMC );

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glDisable( GL_LIGHTING );
			glDisable( GL_TEXTURE_2D );

			glEnable( GL_COLOR_MATERIAL );
			glEnable( GL_DEPTH_TEST );
			glEnable( GL_CULL_FACE );

			glViewport( 0, 0, width, height );

			glMatrixMode( GL_PROJECTION );
			glLoadMatrixf( glm::value_ptr( _proj ) );

			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			glClearColor( 0, 0, 0.15f, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			//glMultMatrixf( glm::value_ptr( _view ) );
			glMultMatrixf( glm::value_ptr( _trackball->mat() ) );

			//program->activate();
			//program->setUniformTex2D( "tex", texName, 0 );

			glLineWidth( 2.0f );
			drawGrid();

			glLineWidth( 3.0f );
			drawAxes();

			onDraw( _sf );

			//quad->draw( program );

			//program->deactivate();

			lateDraw( _sf );
		}
		glPopAttrib();

		if( fb )
			fb->deactivate();
	}

	void FrameDrawer3D::onDraw( const SampleFrame *sf )
	{}

	void FrameDrawer3D::lateDraw( const SampleFrame *sf )
	{}

	bool FrameDrawer3D::mouseDown( int button, int mods, bool imGuiHandled )
	{
		FrameDrawer::mouseDown( button, mods, imGuiHandled );

		if( imGuiHandled )
			return false;

		return _trackball->mouseDown( button, mods, imGuiHandled );
	}

	bool FrameDrawer3D::mouseUp( int button, int mods, bool imGuiHandled )
	{
		FrameDrawer::mouseUp( button, mods, imGuiHandled );

		if( imGuiHandled )
			return false;

		return _trackball->mouseUp( button, mods, imGuiHandled );
	}

	void FrameDrawer3D::mouseMotion( const glm::vec2 &pos )
	{
		FrameDrawer::mouseMotion( pos );

		_trackball->mouseMotion( pos );
	}

	bool FrameDrawer3D::scroll( const glm::vec2 &offset, bool imGuiHandled )
	{
		FrameDrawer::scroll( offset, imGuiHandled );

		if( imGuiHandled )
			return false;

		return _trackball->scroll( offset, imGuiHandled );
	}

	bool FrameDrawer3D::charDown( unsigned char c, bool imGuiHandled )
	{
		FrameDrawer::charDown( c, imGuiHandled );

		if( imGuiHandled )
			return false;

		return _trackball->charDown( c, imGuiHandled );
	}

	bool FrameDrawer3D::keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled )
	{
		FrameDrawer::keyDown( key, scanCode, action, mods, imGuiHandled );

		if( imGuiHandled )
			return false;

		return _trackball->keyDown( key, scanCode, action, mods, imGuiHandled );
	}

	bool FrameDrawer3D::loadFromJSON( const nlohmann::json &j )
	{
		bool ret = FrameDrawer::loadFromJSON( j );

		load<glm::mat4>( j, "p", _proj );
		//_trackball->set( load<glm::mat4>( j, "mv" ) );	//TODO: setting from matrix not implemented

		return ret;
	}

	bool FrameDrawer3D::saveToJSON( nlohmann::json &j ) const
	{
		bool ret = FrameDrawer::saveToJSON( j );

		save( j, "p", _proj );
		//save( j, "mv", _trackball->mat() );	//TODO: setting from matrix not implemented, so don't bother saving

		return ret;
	}








	HistoryDrawerMap::HistoryDrawerMap( size_t bufferSize ) :
		FrameDrawer2D(),
		_bufferSize( bufferSize ),
		_frameWidth( 0 ),
		_frameDepth( 0 )
	{}

	HistoryDrawerMap::~HistoryDrawerMap()
	{
		clear();
	}

	void HistoryDrawerMap::clear()
	{
		//for( auto sf : _sfs )
		//	safeDelete( sf );
		//_sfs.clear();

		_frameWidth = 0;
		_frameDepth = 0;
	}

	bool HistoryDrawerMap::update( const SampleFrame *sf )
	{
		if( !sf )
			return false;

		if( !_bufferSize )
			return false;

		if( sf->height() > 1 )
		{
			std::cerr << "<warning> drawing history of multi-raw frame not supported" << std::endl;
			return false;
		}

		_frameDepth = sf->depth();
		if( _frameDepth > 3 || !_frameDepth )	//can only render in 3-channel color at max
		{
			std::cerr << "<warning> drawing history of frame with depth of " << _frameDepth << " not supported" << std::endl;
			return false;
		}

		_frameWidth = sf->width();
		//if( _sfs.size() )
		//{
		//	if( ( _sfs.front()->width() != width ) || ( _sfs.front()->depth() != depth ) )
		//	{
		//		std::cout << "dimensions changed, have to clear history" << std::endl;
		//		clear();
		//	}
		//}

		size_t size = _bufferSize * _frameWidth * _frameDepth;
		if( _buffer.size() != size )
			_buffer.resize( size );

		//_sfs.push_back( new SampleFrame( *sf ) );

		/*while( _sfs.size() > _bufferSize )
		{
			safeDelete( _sfs.front() );
			_sfs.pop_front();
		}*/

		memcpy( &_buffer[0], &_buffer[_frameWidth * _frameDepth], _frameWidth * _frameDepth * ( _bufferSize - 1 ) * sizeof( float ) );
		memcpy( &_buffer[_frameWidth * _frameDepth * ( _bufferSize - 1 )], sf->values(), _frameWidth * _frameDepth * sizeof( float ) );

		if( !checkTexture( _frameWidth, _bufferSize, _frameDepth ) )
			return false;

		return true;
	}

	void HistoryDrawerMap::draw( FrameBuffer *fb, size_t width, size_t height )
	{
		if( _texName == ~0x00 )
		{
			std::cerr << "<error> texture not initialized" << std::endl;
			return;
		}

		if( fb )
			fb->activate();

		glBindTexture( GL_TEXTURE_2D, _texName );
		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, _texWidth, _texHeight, _texFormat, _texType, &_buffer[0] );
		glBindTexture( GL_TEXTURE_2D, 0 );

		//size_t depth = ( _sfs. sf->depth();
		Program *program = ( _frameDepth > 1 ? programMC : programSC );

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glViewport( 0, 0, width, height );

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			glClearColor( 1, 0, 1, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			glDisable( GL_DEPTH_TEST );
			glDisable( GL_CULL_FACE );
			glDisable( GL_LIGHTING );

			glEnable( GL_COLOR_MATERIAL );
			glEnable( GL_TEXTURE_2D );

			program->activate();
			program->setUniformTex2D( "tex", _texName, 0 );

			quad->draw( program );

			program->deactivate();

			lateDraw( _sf );
		}
		glPopAttrib();

		if( fb )
			fb->deactivate();
	}

	bool HistoryDrawerMap::drawUI()
	{
		bool ret = FrameDrawer::drawUI();

		int s = _bufferSize;
		if( ImGui::SliderInt( "buffer size", &s, 1, 512 ) )
			_bufferSize = s;

		return ret;
	}

	bool HistoryDrawerMap::loadFromJSON( const nlohmann::json &j )
	{
		bool ret = FrameDrawer::loadFromJSON( j );

		load<size_t>( j, "bufferSize", _bufferSize );

		return ret;
	}

	bool HistoryDrawerMap::saveToJSON( nlohmann::json &j ) const
	{
		bool ret = FrameDrawer::saveToJSON( j );

		save( j, "bufferSize", _bufferSize );

		return ret;
	}





	PointCloudDrawer::PointCloudDrawer() :
		FrameDrawer3D()
	{}

	PointCloudDrawer::~PointCloudDrawer()
	{}

	void PointCloudDrawer::onDraw( const SampleFrame *sf )
	{
		if( !sf )
			return;

		if( sf->depth() != 3 )
		{
			std::cerr << "<error> input must be 3-channel frame" << std::endl;
			return;
		}

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glPointSize( 2 );
			glColor3fv( glm::value_ptr( yellow() ) );

			glEnableClientState( GL_VERTEX_ARRAY );

			glVertexPointer( 3, GL_FLOAT, 0, sf->values() );
			glDrawArrays( GL_POINTS, 0, sf->width() * sf->height() );

			glDisableClientState( GL_VERTEX_ARRAY );
		}
		glPopAttrib();
	}
}
#endif