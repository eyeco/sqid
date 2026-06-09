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

#include <common.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <string>

struct GLFWwindow;

namespace sqid
{
	class IniFile;
	class PluginLoader;

	class UIStyle;
	class SceneGraph;

	SQID_API double SQID_API_CALL getAppTime();

#ifdef __SUPPORT_GUI
	class SceneGraphDrawer;
#endif

	class SQID_API Application :
#ifdef __SUPPORT_GUI
		public MouseEventHandler,
#endif
		public KeyEventHandler
	{
		friend SQID_API Application& SQID_API_CALL App();

	private:
		bool _quit;

		std::string _appTitle;
		std::string _sceneName;

		UIStyle *_uis;

		SceneGraph *_sg;

#ifdef __SUPPORT_GUI
		float _fps;
		unsigned int _frameCntr;
		double _timeAccu;

		//stupid ImGui is stupid. have to use flag.
		bool _imGuiInitialized;

		bool _useGui;
		bool _uiActive;

		bool _drawStats;
		bool _drawGrid;

		bool _drawStatusBar;

		float _ar;

		//TODO: replace this by trackball2d
		std::vector<bool> _mousePressed;
		bool _firstFrame;
		glm::vec2 _mousePos;
		glm::vec2 _oldMousePos;

		bool _autoSave;

		glm::ivec2 _windowPos;
		glm::ivec2 _windowSize;

		glm::mat4 _mvCanvas;
		glm::mat4 _mvCanvasInv;

		IniFile *_ini;

		SceneGraphDrawer *_sgd;

		GLFWwindow *_glWindow;

		const float _scaleSpeed = 0.1f;
		const float _minScale = 0.25f;
		const float _maxScale = 2.0f;
#endif

		PluginLoader *_plugins;

		void logAppInfo();

		void updateWindowTitle();

		void load();
		void save( bool saveScene = true );

		void idle();
		void display();

#ifdef __SUPPORT_GUI
		void drawGrids();

		void initImGui();
		void initGL( int argc, char **argv );

		void saveIni();
#endif

		explicit Application( const std::string &appTitle );

	public:
		~Application();

		static Application *create( const std::string &appTitle );

		void init( bool useGui, const std::string &sceneName );
		void run( int argc, char **argv );

		void update();

		void quit() { _quit = true; }

		//bool getQuit() const { return _quit; }
		const std::string &getSceneName() const { return _sceneName; }

		std::string workingDirectory() const;
		std::string executableDirectory() const;

		virtual bool keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled );
		virtual bool charDown( unsigned char c, bool imGuiHandled );

#ifdef __SUPPORT_GUI
		void moved( int x, int y );
		void center( const glm::vec2 &c );
		void resized( int width, int height );

		virtual bool mouseDown( int button, int mods, bool imGuiHandled );
		virtual bool mouseUp( int button, int mods, bool imGuiHandled );
		virtual void mouseMotion( const glm::vec2 &pos );

		virtual bool scroll( const glm::vec2 &offset, bool imGuiHandled );

		bool getAutoSave() const { return _autoSave; }
		//void setAutoSave( bool autoSave ) { _autoSave = autoSave; }

		bool getUseGui() const { return _useGui; }
		bool getUIActive() const { return _uiActive; }

		bool getDrawStats() const { return _drawStats; }
		void setDrawStats( bool draw ) { _drawStats = draw; }

		bool getDrawGrid() const { return _drawGrid; }
		void setDrawGrid( bool draw ) { _drawGrid = draw; }

		bool getDrawStatusBar() const { return _drawStatusBar; }
		void setDrawStatusBar( bool draw ) { _drawStatusBar = draw; }

		const glm::ivec2 &getWindowPos() const { return _windowPos; }
		const glm::ivec2 &getWindowSize() const { return _windowSize; }

		float getAR() const { return _ar; }

		//TODO: replace this by trackball2d
		const glm::mat4 &getMVCanvas() const { return _mvCanvas; }
		const glm::mat4 &getMVCanvasInv() const { return _mvCanvasInv; }

		void writeToClipboard( const std::string &str );
		std::string readFromClipboard();

		bool getMousePressed( unsigned int i ) const { return ( i < _mousePressed.size() ? _mousePressed[i] : false ); }
#else
		//bool getAutoSave() const { return false; }
		//void setAutoSave( bool autoSave ) {}

		bool getUseGui() const { return false; }
#endif
	};
}