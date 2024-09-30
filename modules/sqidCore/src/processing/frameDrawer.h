/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the YPX Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <config.h>

#ifdef __SUPPORT_GUI
#include <guid.h>
#include <common.h>

#include <nlohmann/json.hpp>

namespace YPX
{
	class Shader;
	class Program;
	class SampleFrame;
	class FrameBuffer;
	class VertexBuffer;
	class Trackball3D;

	class SampleFrame;

#define DECLARE_FRAMEDRAWER_DESC \
	virtual const GUID &getClassID() const; \
	virtual const char *getName() const; \
	static const GUID &ClassID(); \
	static const char *Name();

#define DEFINE_FRAMEDRAWER_DESC( _TYPE, _NAME, _GUID ) \
	const char *_TYPE::Name() { return _NAME; } \
	const char *_TYPE::getName() const { return _NAME; } \
	const GUID &_TYPE::ClassID() { static const GUID clsID = guidFromString( _GUID ); return clsID; } \
	const GUID &_TYPE::getClassID() const { return _TYPE::ClassID(); }

	class YPX_API FrameDrawer :
		public MouseEventHandler,
		public KeyEventHandler
	{
	protected:
		bool _flipRB;

		size_t _rtSize;

		SampleFrame *_sf;
		FrameBuffer *_fb;

	public:
		explicit FrameDrawer( size_t rtSize, bool flipRB = true ); //we are flipping by default, as we're working with OpenCV matrices heavily, which use BGR order
		virtual ~FrameDrawer();

		virtual bool update( const SampleFrame *sf );

		void draw();
		virtual void draw( FrameBuffer *fb, size_t width, size_t height ) = 0;

		virtual bool drawUI();

		virtual bool loadFromJSON( const nlohmann::json &j );
		virtual bool saveToJSON( nlohmann::json &j ) const;

		GLuint getRTName() const;
		size_t getRTSize() const { return _rtSize; }

		static bool init();
		static bool uninit();

		virtual const GUID &getClassID() const = 0;
		virtual const char *getName() const = 0;
	};

	class FrameDrawerLines : public FrameDrawer
	{
	protected:
		bool _autoScale;
		float _scale;
		float _offset;

		bool _drawInfo;

		float _lineWidth;

		std::vector<GLfloat> _vertices;

		bool _isSelected;
		bool _isMouseDown;
		glm::vec2 _selectionPos;

		std::vector<GLfloat> _selectionVertices;
		std::vector<GLfloat> _inspectedVertices;

		virtual void checkVertices( size_t size );

	public:
		explicit FrameDrawerLines( float lineWidth = 1.5f );
		virtual ~FrameDrawerLines();

		virtual void draw( FrameBuffer *fb, size_t width, size_t height );

		virtual bool drawUI();

		virtual bool mouseDown( int button, int mods, bool imGuiHandled );
		virtual bool mouseUp( int button, int mods, bool imGuiHandled );
		virtual void mouseMotion( const glm::vec2 &pos );

		virtual bool scroll( const glm::vec2 &offset, bool imGuiHandled );

		virtual bool keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled );
		virtual bool charDown( unsigned char c, bool imGuiHandled );

		virtual bool loadFromJSON( const nlohmann::json &j );
		virtual bool saveToJSON( nlohmann::json &j ) const;

		DECLARE_FRAMEDRAWER_DESC
	};

	class HistoryDrawerLines : public FrameDrawerLines
	{
	protected:
		size_t _bufferSize;

		std::list<SampleFrame*> _sfs;

		void clear();
		virtual void checkVertices( size_t size );

	public:
		explicit HistoryDrawerLines( size_t bufferSize = 64, float lineWidth = 1.5f );
		virtual ~HistoryDrawerLines();

		virtual bool update( const SampleFrame *sf );

		virtual void draw( FrameBuffer *fb, size_t width, size_t height );

		virtual bool drawUI();

		virtual bool loadFromJSON( const nlohmann::json &j );
		virtual bool saveToJSON( nlohmann::json &j ) const;

		DECLARE_FRAMEDRAWER_DESC
	};

	class FrameDrawer2D : public FrameDrawer
	{
	protected:
		unsigned int _texWidth;
		unsigned int _texHeight;
		unsigned int _texDepth;

		GLenum _texFormat;
		GLenum _texType;

		GLuint _texName;

		bool checkTexture( size_t width, size_t height, size_t depth );
		bool checkTexture( const SampleFrame *sf );
		void releaseTexture();

		virtual void lateDraw( const SampleFrame *sf );

	public:
		FrameDrawer2D();
		virtual ~FrameDrawer2D();

		virtual bool update( const SampleFrame *sf );

		virtual void draw( FrameBuffer *fb, size_t width, size_t height );

		virtual unsigned int getWidth() const { return _texWidth; }
		virtual unsigned int getHeight() const { return _texHeight; }

		DECLARE_FRAMEDRAWER_DESC
	};

	class FrameDrawer3D : public FrameDrawer
	{
	private:
		glm::mat4 _proj;

		Trackball3D *_trackball;

	protected:

		virtual void onDraw( const SampleFrame *sf );
		virtual void lateDraw( const SampleFrame *sf );

	public:
		FrameDrawer3D();
		virtual ~FrameDrawer3D();

		virtual bool update( const SampleFrame *sf );

		virtual void draw( FrameBuffer *fb, size_t width, size_t height );

		virtual bool mouseDown( int button, int mods, bool imGuiHandled );
		virtual bool mouseUp( int button, int mods, bool imGuiHandled );
		virtual void mouseMotion( const glm::vec2 &pos );

		virtual bool scroll( const glm::vec2 &offset, bool imGuiHandled );

		virtual bool keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled );
		virtual bool charDown( unsigned char c, bool imGuiHandled );

		virtual bool loadFromJSON( const nlohmann::json &j );
		virtual bool saveToJSON( nlohmann::json &j ) const;

		DECLARE_FRAMEDRAWER_DESC
	};
	
	class HistoryDrawerMap : public FrameDrawer2D
	{
	protected:
		size_t _bufferSize;
	
		size_t _frameWidth;
		size_t _frameDepth;

		std::vector<float> _buffer;
		//std::list<SampleFrame*> _sfs;

		void clear();

	public:
		explicit HistoryDrawerMap( size_t bufferSize = 64 );
		virtual ~HistoryDrawerMap();

		virtual bool update( const SampleFrame *sf );

		virtual void draw( FrameBuffer *fb, size_t width, size_t height );

		virtual bool drawUI();

		virtual bool loadFromJSON( const nlohmann::json &j );
		virtual bool saveToJSON( nlohmann::json &j ) const;

		DECLARE_FRAMEDRAWER_DESC
	};
	
	class PointCloudDrawer : public FrameDrawer3D
	{
	protected:
		virtual void onDraw( const SampleFrame *sf );

	public:
		PointCloudDrawer();
		virtual ~PointCloudDrawer();

		DECLARE_FRAMEDRAWER_DESC
	};
}
#endif