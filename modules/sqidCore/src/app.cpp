/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <app.h>

#include <iostream>
#include <filesystem>

#include "pluginLoader.h"

#include "fileIO/iniFile.h"

#include "processing/sceneGraph.h"

#include "processing/opFactory.h"
#include "processing/coreOps.h"

#ifdef __SUPPORT_GUI
#include "drawing/sceneUI/opDrawer.h"
#include "drawing/sceneUI/sceneGraphDrawer.h"
#include <clipboardxx.hpp>
#endif


#ifdef _WIN32
#include <Windows.h>
#include <gl/glew.h>
#include <gl/wglew.h>
#include <conio.h>
#elif __GNUC__
#include <GL/glew.h>
#include <unistd.h>
#include <signal.h>
#endif

#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <commonImGui.h>

#include <glm/gtx/transform.hpp>


#ifdef __SUPPORT_GUI
#include "drawing/sceneUI/uiStyle.h"
#include <drawing/frameDrawer.h>

#ifdef _WIN32
#pragma comment( lib, "glfw3.lib" )
#ifdef _DEBUG
#pragma comment( lib, "glew32d.lib" )
#else
#pragma comment( lib, "glew32.lib" )
#endif
#endif

#endif

namespace sqid
{
	Application *app = nullptr;

	double appTime = 0.0;

	SQID_API double SQID_API_CALL getAppTime() { return appTime; }

	SQID_API Application& SQID_API_CALL App()
	{
		if( !sqid::app )
			throw std::runtime_error( "app not yet initialized" );
		return *sqid::app;
	}

#ifdef __SUPPORT_GUI

	void Application::drawGrids()
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glDisable( GL_DEPTH_TEST );
			glDisable( GL_TEXTURE_2D );
			glDisable( GL_LIGHTING );
			glEnable( GL_COLOR_MATERIAL );

			std::vector<glm::vec2> verts;

			glEnableClientState( GL_VERTEX_ARRAY );

			glDisableClientState( GL_COLOR_ARRAY );
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );

			glm::vec4 upperLeftSS( 0, 0, 0, 1 );
			glm::vec4 lowerRightSS( _windowSize.x, _windowSize.y, 0, 1 );

			glm::vec4 upperLeftWS = _mvCanvasInv * upperLeftSS;
			glm::vec4 lowerRightWS = _mvCanvasInv * lowerRightSS;

			if( _sgd->getEditMode() )
				glColor4f( _uis->GridColorEdit[0], _uis->GridColorEdit[1], _uis->GridColorEdit[2], 1.0f );
			else
				glColor4f( _uis->GridColor[0], _uis->GridColor[1], _uis->GridColor[2], 1.0f );

			for( int p = 3; p > 0; p-- )
			{
				int res = pow( 10, p );

				if( res * _mvCanvasInv[0][0] < 8 )
					continue;

				verts.clear();
				verts.clear();

				int i = upperLeftWS.x / res;
				float x = i * res;
				float y0 = upperLeftWS.y;
				float y1 = lowerRightWS.y;

				while( x < lowerRightWS.x )
				{
					verts.push_back( ( glm::vec4( x, y0, 0, 1 ) ).xy );
					verts.push_back( ( glm::vec4( x, y1, 0, 1 ) ).xy );

					x += res;
				}

				int j = upperLeftWS.y / res;
				float y = j * res;
				float x0 = upperLeftWS.x;
				float x1 = lowerRightWS.x;

				while( y < lowerRightWS.y )
				{
					verts.push_back( ( glm::vec4( x0, y, 0, 1 ) ).xy );
					verts.push_back( ( glm::vec4( x1, y, 0, 1 ) ).xy );

					y += res;
				}

				if( verts.size() )
				{
					glLineWidth( p );

					glVertexPointer( 2, GL_FLOAT, 0, &verts[0] );

					glDrawArrays( GL_LINES, 0, verts.size() );
				}
			}

			glDisableClientState( GL_VERTEX_ARRAY );
			glDisableClientState( GL_COLOR_ARRAY );
		}
		glPopAttrib();
	}

	bool Application::keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled )
	{
		bool handled = false;
		if( _useGui )
		{
			handled = _sgd->keyDown( key, scanCode, action, mods, imGuiHandled );

			if( !imGuiHandled && !handled )
			{
				if( action == GLFW_PRESS )
				{
					switch( key )
					{
					case GLFW_KEY_S:
						if( mods & GLFW_MOD_CONTROL )
							save();
						break;
					case GLFW_KEY_Q:
						if( mods & GLFW_MOD_CONTROL )
							quit();
						break;
#ifdef __QUIT_WITH_ESCAPE
					case GLFW_KEY_ESCAPE:
						quit();
						break;
#endif
					}
				}
			}
		}
		else
		{
			//switch( key )
			//{
			//}
		}

		return handled;
	}
#endif


	bool Application::charDown( unsigned char c, bool imGuiHandled )
	{
		bool handled = false;

#ifdef __SUPPORT_GUI
		if( !imGuiHandled )
		{
			if( _uiActive )
			{
				switch( c )
				{
				case 'r':
					_mvCanvas = glm::identity<glm::mat4>();
					_mvCanvasInv = glm::identity<glm::mat4>();
					handled = true;
					break;
				}
			}
			
			switch( c )
			{
			case ' ':
				_uiActive = !_uiActive;
				std::cout << ( _uiActive ? "activated" : "deactivated" ) << " UI" << std::endl;
				handled = true;
				break;
			}
		}

		if( _useGui )
		{
			handled = _sgd->charDown( c, imGuiHandled );

			if( !handled && !imGuiHandled )
			{
				switch( c )
				{
				case 'd':
					if( _sgd )
					{
						_sgd->setDrawDebug( !_sgd->getDrawDebug() );
						if( _sg && _sgd->getDrawDebug() )
							_sg->printOrder();
					}
					break;
				case 'a':
					if( _sgd )
					{
						Rect bb;
						if( _sgd->getBB( bb ) )
							App().center( bb.Center() );
					}
					break;
				case 'f':
					if( _sgd )
					{
						Rect bb;
						if( _sgd->getBBSelected( bb ) )
							center( bb.Center() );
					}
					break;
				default:
					break;
				}
			}
		}
		else
#endif
		{
			//switch( c )
			//{
			//}
		}

		return handled;
	}

	void Application::update()
	{
		static auto prevTime = std::chrono::system_clock::now();
		auto currentTime = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = currentTime - prevTime;
		prevTime = currentTime;

		double dt = diff.count();
		_timeAccu += dt;
		appTime += dt;

		if( _timeAccu >= 1.0 )
		{
			_fps = _frameCntr / _timeAccu;
			_timeAccu = 0;
			_frameCntr = 0;
		}

		if( _sg )
			_sg->update( (float) dt );
	}

#ifdef __SUPPORT_GUI
	/*
	 *  @param[in] error An [error code](@ref errors).
	 *  @param[in] description A UTF-8 encoded string describing the error.
	 */
	void onError( int error, const char *desc )
	{
		std::cerr << "<error> GLFW err #" << error << ": \"" << desc << "\"" << std::endl;
	}

	/*
	 *  @param[in] xpos The new x-coordinate, in screen coordinates, of the
	 *  upper-left corner of the content area of the window.
	 *  @param[in] ypos The new y-coordinate, in screen coordinates, of the
	 *  upper-left corner of the content area of the window.
	 */
	void onWindowMoved( GLFWwindow *window, int x, int y )
	{
		app->moved( x, y );
	}

	/*
	 *  @param[in] width The new width, in screen coordinates, of the window.
	 *  @param[in] height The new height, in screen coordinates, of the window.
	 */
	void onWindowResized( GLFWwindow *window, int width, int height )
	{
		app->resized( width, height );
	}

	/*
	 *  @param[in] window The window that the user attempted to close.
	 */
	void onWindowClosed( GLFWwindow *window )
	{
		app->quit();
	}

	/*
	 *  @param[in] window The window that received the event.
	 *  @param[in] key The [keyboard key](@ref keys) that was pressed or released.
	 *  @param[in] scancode The system-specific scancode of the key.
	 *  @param[in] action `GLFW_PRESS`, `GLFW_RELEASE` or `GLFW_REPEAT`.
	 *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
	 *  held down.
	 */
	void onKey( GLFWwindow *window, int key, int scanCode, int action, int mods )
	{
		bool imGuiHandled = ( ImGui::GetIO().WantCaptureKeyboard && app->getUIActive() );
		
		app->keyDown( key, scanCode, action, mods, imGuiHandled );

		if( app->getUIActive() )
			ImGui_ImplGlfw_KeyCallback( window, key, scanCode, action, mods );
	}

	/*
	 *@param[in] codepoint The Unicode code point of the character.
	 */
	void onChar( GLFWwindow *window, unsigned int c )
	{
		bool imGuiHandled = ( ImGui::GetIO().WantCaptureKeyboard && app->getUIActive() );

		app->charDown( c, imGuiHandled );

		if( app->getUIActive() )
			ImGui_ImplGlfw_CharCallback( window, c );
	}

	/*
	 *  @param[in] codepoint The Unicode code point of the character.
	 *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
	 */
	void onCharMods( GLFWwindow *window, unsigned int c, int mods )
	{
	}

	/*
	 *  @param[in] button The [mouse button](@ref buttons) that was pressed or
	 *  released.
	 *  @param[in] action One of `GLFW_PRESS` or `GLFW_RELEASE`.
	 *  @param[in] mods Bit field describing which [modifier keys](@ref mods) were
	 *  held down.
	 */
	void onMouseButton( GLFWwindow *window, int button, int action, int mods )
	{
		bool imGuiHandled = ( ImGui::GetIO().WantCaptureMouse && app->getUIActive() );

		//if( ImGui::GetIO().WantCaptureMouse && action == GLFW_PRESS )
		//	std::cout << "ImGui wants to capture mouse button press" << std::endl;

		bool handled = false;
		if( action == GLFW_PRESS )
			handled |= app->mouseDown( button, mods, imGuiHandled );
		else if( action == GLFW_RELEASE )
			handled |= app->mouseUp( button, mods, imGuiHandled );

		if( app->getUIActive() )
			ImGui_ImplGlfw_MouseButtonCallback( window, button, action, mods );
	}

	/*
	 *  @param[in] xpos The new cursor x-coordinate, relative to the left edge of
	 *  the content area.
	 *  @param[in] ypos The new cursor y-coordinate, relative to the top edge of the
	 *  content area.
	 */
	void onCursorMoved( GLFWwindow *window, double x, double y )
	{
		app->mouseMotion( glm::vec2( x, y ) );
	}

	/*
	 *  @param[in] xoffset The scroll offset along the x-axis.
	 *  @param[in] yoffset The scroll offset along the y-axis.
	 */
	void onScroll( GLFWwindow *window, double xoffset, double yoffset )
	{
		bool imGuiHandled = ( ImGui::GetIO().WantCaptureMouse && app->getUIActive() );

		bool handled = false;

		handled |= app->scroll( glm::vec2( xoffset, yoffset ), imGuiHandled );

		if( app->getUIActive() )
			ImGui_ImplGlfw_ScrollCallback( window, xoffset, yoffset );
	}

	void Application::display()
	{
		_frameCntr++;

		if( _sgd->getEditMode() )
			glClearColor( _uis->BackgroundColorEdit[0], _uis->BackgroundColorEdit[1], _uis->BackgroundColorEdit[2], 1.0f );
		else
			glClearColor( _uis->BackgroundColor[0], _uis->BackgroundColor[1], _uis->BackgroundColor[2], 1.0f );

		glViewport( 0, 0, App().getWindowSize().x, App().getWindowSize().y );

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		gluOrtho2D( -1.0f, 1.0f, -1.0f / App().getAR(), 1.0f / App().getAR() );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		if( _sgd )
			_sgd->preDraw();

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		gluOrtho2D( 0.0f, App().getWindowSize().x, App().getWindowSize().y, 0.0f );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		glMultMatrixf( glm::value_ptr( App().getMVCanvas() ) );

		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			if( App().getDrawGrid() )
				drawGrids();

			if( _sgd )
				_sgd->draw();
		}
		glPopAttrib();

		static auto prevTime = std::chrono::system_clock::now();
		auto currentTime = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = currentTime - prevTime;
		prevTime = currentTime;

		double dt = diff.count();

		if( App().getUIActive() )
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();

			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = glm2im( App().getWindowSize() );
			if( io.DeltaTime < std::numeric_limits<float>::epsilon() )
				io.DeltaTime = (float) dt;
			ImGui::NewFrame();

			if( ImGui::BeginMainMenuBar() )
			{
				if( ImGui::BeginMenu( "File" ) )
				{
					//TODO
					if( ImGui::MenuItem( "New", "Ctrl+N", false, false ) )
					{
					}

					//TODO:
					if( ImGui::MenuItem( "Open", "Ctrl+O", false, false ) )
					{
					}

					if( ImGui::MenuItem( "Save", "Ctrl+S" ) )
						App().save();

					//TODO:
					if( ImGui::MenuItem( "Save As..", nullptr, false, false ) )
					{
					}

					if( ImGui::MenuItem( "Auto-save scene", NULL, _autoSave ) )
					{
						_autoSave = !_autoSave;
					}

					if( ImGui::MenuItem( "Quit", "Ctrl+Q" ) )
					{
						App().quit();
					}

					ImGui::EndMenu();
				}

				if( ImGui::BeginMenu( "Edit" ) )
				{
					if( ImGui::MenuItem( "Edit mode", "Ctrl+E", _sgd->getEditMode() ) )
					{
						_sgd->setEditMode( !_sgd->getEditMode() );
					}

					if( ImGui::MenuItem( "Copy", "Ctrl+C" ) )
					{
						_sgd->copyToClipboard();
					}

					if( ImGui::MenuItem( "Paste", "Ctrl+V" ) )
					{
						_sgd->pasteFromClipboard();
					}

					ImGui::EndMenu();
				}

				if( ImGui::BeginMenu( "View" ) )
				{
					//TODO
					bool b = App().getDrawStats();
					if( ImGui::MenuItem( "Display stats window", nullptr, &b ) )
						App().setDrawStats( b );

					b = App().getDrawGrid();
					if( ImGui::MenuItem( "Draw grid", nullptr, &b ) )
						App().setDrawGrid( b );

					if( ImGui::MenuItem( "Display debug info", NULL, _sgd->getDrawDebug() ) )
						_sgd->setDrawDebug( !_sgd->getDrawDebug() );

					b = App().getDrawStatusBar();
					if( ImGui::MenuItem( "Status bar", nullptr, &b ) )
						App().setDrawStatusBar( b );

					ImGui::EndMenu();
				}

				/*
				if( ImGui::BeginMenu( "Tools" ) )
				{
					ImGui::EndMenu();
				}
				*/

				ImGui::EndMainMenuBar();
			}

			if( App().getDrawStatusBar() )
			{
				ImGui::SetNextWindowPos( ImVec2( 0, App().getWindowSize().y - 25 ) );
				ImGui::SetNextWindowSize( ImVec2( App().getWindowSize().x, 25 ) );
				ImGui::Begin( "##status", nullptr,
					ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings );

				char tempStr[128];
				sprintf( tempStr, "fps: %.02f", _fps );
				ImGui::Text( "%s", tempStr );
				ImGui::End();
			}

			if( _sgd )
				_sgd->drawUI();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
		}

		glfwMakeContextCurrent( _glWindow );
		glfwSwapBuffers( _glWindow );

		checkForGLError();
	}

	void Application::idle()
	{
		glfwPollEvents();

		update();
		display();
	}

	void Application::initImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void) io;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL( _glWindow, false );
		ImGui_ImplOpenGL3_Init();

		ImGuiStyle& style = ImGui::GetStyle();

		float alpha = 1.0f;
		// https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9#file-imguiutils-h-L9-L93
		// light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
		style.Alpha = 1.0f;
		style.FrameRounding = 3.0f;
		style.Colors[ImGuiCol_Text] = ImVec4( 0.00f, 0.00f, 0.00f, 1.00f );
		style.Colors[ImGuiCol_TextDisabled] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
		style.Colors[ImGuiCol_WindowBg] = ImVec4( 0.94f, 0.94f, 0.94f, 0.94f );
		style.Colors[ImGuiCol_ChildBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
		style.Colors[ImGuiCol_PopupBg] = ImVec4( 1.00f, 1.00f, 1.00f, 0.94f );
		style.Colors[ImGuiCol_Border] = ImVec4( 0.00f, 0.00f, 0.00f, 0.39f );
		style.Colors[ImGuiCol_BorderShadow] = ImVec4( 1.00f, 1.00f, 1.00f, 0.10f );
		style.Colors[ImGuiCol_FrameBg] = ImVec4( 1.00f, 1.00f, 1.00f, 0.94f );
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.40f );
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4( 0.26f, 0.59f, 0.98f, 0.67f );
		style.Colors[ImGuiCol_TitleBg] = ImVec4( 0.96f, 0.96f, 0.96f, 1.00f );
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 1.00f, 1.00f, 1.00f, 0.51f );
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4( 0.82f, 0.82f, 0.82f, 1.00f );
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4( 0.86f, 0.86f, 0.86f, 1.00f );
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.98f, 0.98f, 0.98f, 0.53f );
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.69f, 0.69f, 0.69f, 1.00f );
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.59f, 0.59f, 0.59f, 1.00f );
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 0.49f, 0.49f, 0.49f, 1.00f );
		style.Colors[ImGuiCol_CheckMark] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
		style.Colors[ImGuiCol_SliderGrab] = ImVec4( 0.24f, 0.52f, 0.88f, 1.00f );
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
		style.Colors[ImGuiCol_Button] = ImVec4( 0.26f, 0.59f, 0.98f, 0.40f );
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
		style.Colors[ImGuiCol_ButtonActive] = ImVec4( 0.06f, 0.53f, 0.98f, 1.00f );
		style.Colors[ImGuiCol_Header] = ImVec4( 0.26f, 0.59f, 0.98f, 0.31f );
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
		style.Colors[ImGuiCol_HeaderActive] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4( 1.00f, 1.00f, 1.00f, 0.50f );
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.67f );
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
		style.Colors[ImGuiCol_Tab] = ImVec4( 0.26f, 0.59f, 0.98f, 0.31f );
		style.Colors[ImGuiCol_TabHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
		style.Colors[ImGuiCol_TabActive] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4( 0.26f, 0.59f, 0.98f, 0.31f );
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
		style.Colors[ImGuiCol_PlotLines] = ImVec4( 0.39f, 0.39f, 0.39f, 1.00f );
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4( 0.06f, 0.53f, 0.98f, 1.00f );
		//style.Colors[ImGuiCol_NavHighlight] = ImVec4(  );
		//style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(  );
		//style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(  );
		//style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(  );

		style.WindowRounding = 0;
		style.FrameRounding = 0;
		style.PopupRounding = 0;

		for( int i = 0; i <= ImGuiCol_COUNT; i++ )
		{
			ImVec4& col = style.Colors[i];
			if( col.w < 1.00f )
			{
				col.x *= alpha;
				col.y *= alpha;
				col.z *= alpha;
				col.w *= alpha;
			}
		}

		//stupid ImGui is stupid. have to use flag so it won't crash at shutdown when we did not make it until here.
		_imGuiInitialized = true;

		//_plugins->initImGui();
	}

	void Application::initGL( int argc, char **argv )
	{
		glfwSetErrorCallback( onError );

		if( !glfwInit() )
			throw std::runtime_error( "failed to init glfw" );

		//const char* glsl_version = "#version 130";
		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

		glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
		glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );

		_glWindow = glfwCreateWindow( App().getWindowSize().x, App().getWindowSize().y, _appTitle.c_str(), nullptr, nullptr );
		if( !_glWindow )
			throw std::runtime_error( "failed to create glfw window" );

		glfwMakeContextCurrent( _glWindow );
		glfwSwapInterval( 1 ); // Enable vsync

#ifdef _WIN32
		glfwSetWindowAttrib( _glWindow, GLFW_RESIZABLE, GLFW_TRUE );
#else 
	//TODO
#endif

		glfwSetWindowPos( _glWindow, App().getWindowPos().x, App().getWindowPos().y );
		glfwSetWindowSize( _glWindow, App().getWindowSize().x, App().getWindowSize().y );


		glfwSetWindowPosCallback( _glWindow, onWindowMoved );
		glfwSetWindowSizeCallback( _glWindow, onWindowResized );
		glfwSetKeyCallback( _glWindow, onKey );
		glfwSetCharCallback( _glWindow, onChar );
		glfwSetCharModsCallback( _glWindow, onCharMods );
		glfwSetMouseButtonCallback( _glWindow, onMouseButton );
		glfwSetCursorPosCallback( _glWindow, onCursorMoved );
		glfwSetScrollCallback( _glWindow, onScroll );
		glfwSetWindowCloseCallback( _glWindow, onWindowClosed );

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
		bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
		bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
		bool err = gladLoadGL() == 0;
#else
		bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
		if( err )
			throw std::runtime_error( "Failed to initialize OpenGL loader!" );

		std::cout << "GL version: " << glGetString( GL_VERSION ) << std::endl;
		//TODO: print some more gl infos

#ifdef _WIN32
		//enable v-sync
		wglSwapIntervalEXT( 1 );
#endif

		_uis->init();

		glPointSize( 4 );
		glLineWidth( 1 );

		glDisable( GL_TEXTURE_2D );
		glDisable( GL_LIGHTING );
		glDisable( GL_COLOR_MATERIAL );
		glDisable( GL_CULL_FACE );
		glDisable( GL_DEPTH_TEST );

		initImGui();

		moved( App().getWindowPos().x, App().getWindowPos().y );
		resized( _windowSize.x, _windowSize.y );

		std::cout << "initialized GL" << std::endl;

		if( !initFont( "resources/fonts/courier.ttf", 12, App().getWindowSize().x, App().getWindowSize().y ) )
			std::cerr << "<error> loading font file failed" << std::endl;
	}

	void Application::updateWindowTitle()
	{
		std::string s( _appTitle );
		s.append( ": " );
		s.append( _sceneName );
		glfwSetWindowTitle( _glWindow, s.c_str() );
	}
#endif

	Application *Application::create( const std::string &appTitle )
	{
		if( app )
			throw std::runtime_error( "only one instance of Application allowed" );
		app = new Application( appTitle );
		return app;
	}

	Application::Application( const std::string &appTitle ) :
		_quit( false ),
		_appTitle( appTitle ),
		_sceneName( "" ),
		_sg( nullptr ),
#ifdef __SUPPORT_GUI
		_fps( 0 ),
		_frameCntr( 0 ),
		_timeAccu( 0.0 ),
		_imGuiInitialized( false ),
		_useGui( false ),
		_uiActive( true ),
		_drawStats( true ),
		_drawGrid( true ),
		_drawStatusBar( true ),
		_ar( (float) _windowSize.x / _windowSize.y ),
		_mousePressed( 5 ),
		_firstFrame( true ),
		_autoSave( false ),
		_windowPos( 100, 100 ),
		_windowSize( 1800, 1040 ),
		_mvCanvas( glm::identity<glm::mat4>() ),
		_mvCanvasInv( glm::identity<glm::mat4>() ),
		_ini( nullptr ),
		_uis( nullptr ),
		_sgd( nullptr ),
		_glWindow( nullptr ),
#endif
		_plugins( new PluginLoader() )
	{}

	Application::~Application()
	{
		save( _autoSave );

#ifdef __SUPPORT_GUI
		if( _useGui )
		{
			FrameDrawer::uninit();

			//stupid ImGui is stupid. have to use flag to avoid potential crash.
			if( _imGuiInitialized )
			{
				ImGui_ImplOpenGL3_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();
			}
		}

		if( _glWindow )
		{
			glfwDestroyWindow( _glWindow );
			_glWindow = nullptr;
		}
		glfwTerminate();

		safeDelete( _sgd );

		safeDelete( _uis );
#endif

		safeDelete( _ini );

		safeDelete( _sg );

		opFactory().shutdown();

		_plugins->unload();
		safeDelete( _plugins );

		sqid::app = nullptr;
	}

	void Application::init( bool useGui, const std::string &sceneName )
	{
#ifdef __SUPPORT_GUI
		_useGui = useGui;
#endif
		_sceneName = sceneName;

		logAppInfo();

		//TODO (plugins): list all loaded plugins
		namespace fs = std::filesystem;
		fs::path pluginPath = fs::path( executableDirectory() );// / fs::path( "plugins" );
		_plugins->rescan( pluginPath.string() );

		_plugins->load();
	}

	void Application::run( int argc, char **argv )
	{
		namespace fs = std::filesystem;

		int dirCntr = 0;
		char dir[64];
		do
		{
			sprintf( dir, "%03d", dirCntr++ );
		} while( fs::exists( toRecordingsPath( dir ) ) );

		setRecordingsDirName( toRecordingsPath( dir ).c_str() );

		if( !initCoreOps() )
			std::cerr << "<error> failed to initialize core Ops" << std::endl;
		_plugins->registerOps();

		_sg = new SceneGraph();

		if( _sceneName.size() )
			_sg->load( _sceneName );

		_sg->run();

#ifdef __SUPPORT_GUI
		if( _useGui )
		{
			load();
			
			_uis = new UIStyle();
			initGL( argc, argv );

			FrameDrawer::init();
			
			_sgd = new SceneGraphDrawer( _sg, _uis );
			_sgd->build();
			
			updateWindowTitle();
			
			while( !_quit && !glfwWindowShouldClose( _glWindow ) )
			{
				idle();
			}
		}
		else
#endif
		{
			if( _autoSave )
			{
				std::cerr << "autosave disabled for console mode" << std::endl;
				_autoSave = false;
			}
			
			while( !_quit )
			{
				update();
#ifdef _WIN32
				Sleep( 3 );

				if( _kbhit() )
				{
					charDown( _getch(), false );
				}
#else
				usleep( 3000 );

				if( _kbhit() )
				{
					charDown( getchar(), false );
				}
#endif
			}
		}
	}

	void Application::logAppInfo()
	{
		//TODO: automatic app versioning?
		std::cout << "sqid version " << __VERSION_STRING
			//__VERSION_MAJOR << "." << __VERSION_MINOR << "." << __VERSION_PATCH 
			<< std::endl;

#ifdef _WIN32
		std::cout <<
#ifdef _DEBUG
			"Debug"
#else
			"Release"
#endif				
			<< " build for " <<
#ifdef _M_AMD64 
			"AMD64"
#endif
#ifdef _M_ARM
			"ARM"
#endif
#ifdef _M_IX86
			"x86"
#endif
			<< " with C++ language standard v" << _MSVC_LANG << std::endl;

#ifdef _MSC_VER
#ifdef _MSC_FULL_VER
		std::cout << "built with MSVC v" << _MSC_VER << "(" << _MSC_FULL_VER << ")";
#else
		std::cout << "built with MSVC v" << _MSC_VER;
#endif
#endif //_MSC_VER
#ifdef __GNUC__
		std::cout << "built with GCC v" << __GNUC__ << "." << __GNUC_MINOR__;
#endif //__GNUC__
#ifdef __clang__
		std::cout << "built with clang v" << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#endif //__clang__
#ifdef __MINGW32__
		std::cout << "built with MinGW 32 v" << __MINGW32_MAJOR_VERSION << "." << __MINGW32_MINOR_VERSION;
#endif //__MINGW32
#ifdef __MINGW64__
		std::cout << "built with MinGW 64 v" << __MINGW64_MAJOR_VERSION << "." << __MINGW64_MINOR_VERSION;
#endif //__MINGW64__

#else //_WIN32
		//TODO:
		std::cout << "built for unknown OS";
#endif //_WIN32

		std::cout << " at " << __TIMESTAMP__ << " with: " << std::endl;
		std::cout << "  GUI support           " <<
#ifdef __SUPPORT_GUI
			"YES"
#else
			"NO"
#endif
			<< std::endl;

		std::cout << "  Compression support          " <<
#ifdef __COMPRESSION_SUPPORT
			"YES"
#else
			"NO"
#endif
			<< std::endl;

#ifdef __COMPRESSION_SUPPORT
		std::cout << "    with algorithms: ";
#  ifdef __COMPRESSION_SUPPORT_LZO
		std::cout << "LZO ";
#  endif
#  ifdef __COMPRESSION_SUPPORT_QLZ
		std::cout << "QuickLZ ";
#  endif
#  ifdef __COMPRESSION_SUPPORT_BZ2
		std::cout << "bzip2 ";
#  endif
#  ifdef __COMPRESSION_SUPPORT_ZSTD
		std::cout << "ZStd ";
#  endif
#  ifdef __COMPRESSION_SUPPORT_ZLIB
		std::cout << "deflate ";
#  endif
#  ifdef __COMPRESSION_SUPPORT_LZ4
		std::cout << "LZ4 ";
#  endif
#  ifdef __COMPRESSION_SUPPORT_JPEG
		std::cout << "jpeg ";
#  endif
		std::cout << std::endl;
#endif

		std::cout << "  Bluetooth Classic support    " <<
#ifdef __RFCOMM_SUPPORT
			"YES"
#else
			"NO"
#endif
			<< std::endl;

		std::cout << "  FFTW support                 " <<
#ifdef __FFTW_SUPPORT
			"YES"
#else
			"NO"
#endif
			<< std::endl;
	}

#ifdef __SUPPORT_GUI
	bool Application::mouseDown( int button, int mods, bool imGuiHandled )
	{
		if( _uiActive )
			if( button < _mousePressed.size() )
				_mousePressed[button] = true;

		return _sgd->mouseDown( button, mods, imGuiHandled );
	}

	bool Application::mouseUp( int button, int mods, bool imGuiHandled )
	{
		if( _uiActive )
			if( button < _mousePressed.size() )
				_mousePressed[button] = false;

		return _sgd->mouseUp( button, mods, imGuiHandled );
	}

	void Application::mouseMotion( const glm::vec2 &pos )
	{
		if( _uiActive )
		{
			if( _firstFrame )
				_firstFrame = false;
			else
			{
				if( _mousePressed[1] )
				{
					glm::vec3 t(
						pos.x - _oldMousePos.x,
						pos.y - _oldMousePos.y,
						0
					);

					_mvCanvas = glm::translate( t ) * _mvCanvas;
					_mvCanvasInv = glm::inverse( _mvCanvas );
				}

				_oldMousePos = pos;
			}
			_mousePos = pos;
		}

		_sgd->mouseMotion( pos );
	}

	bool Application::scroll( const glm::vec2 &offset, bool imGuiHandled )
	{
		bool handled = false;

		if( _uiActive && !imGuiHandled )
		{
			float currentScale = _mvCanvas[2][2];
			float targetScale = clamp<float>( currentScale + offset.y * _scaleSpeed, _minScale, _maxScale );

			float scale = targetScale / currentScale;

			_mvCanvas =
				glm::translate( glm::vec3( _mousePos.x, _mousePos.y, 0 ) ) *
				glm::scale( glm::vec3( scale ) ) *
				glm::translate( -glm::vec3( _mousePos.x, _mousePos.y, 0 ) ) *
				_mvCanvas;
			_mvCanvasInv = glm::inverse( _mvCanvas );

			handled = true;
		}

		handled |= _sgd->scroll( offset, imGuiHandled );

		return handled;
	}

	void Application::moved( int x, int y )
	{
		_windowPos.x = x;
		_windowPos.y = y;
	}

	void Application::center( const glm::vec2 &c )
	{
		_mvCanvas = glm::translate( glm::vec3( _windowSize.x * 0.5f - c.x, _windowSize.y * 0.5f - c.y, 0 ) );
		_mvCanvasInv = glm::inverse( _mvCanvas );
	}

	void Application::resized( int width, int height )
	{
		_windowSize.x = width;
		_windowSize.y = height;
		_ar = (float) width / height;

		updateFontWindow( width, height );
	}

	void Application::load()
	{
		std::stringstream sstr;
		sstr << _sceneName << "-app.ini";

		_ini = new IniFile( sstr.str() );
		if( !_ini->read() )
			std::cerr << "<warning> failed to read window.ini" << std::endl;
		else
		{
			IniFile::Section *w = _ini->tryGet( "window" );
			if( w )
			{
				w->tryGet<int>( "posX", _windowPos.x );
				w->tryGet<int>( "posY", _windowPos.y );

				w->tryGet<int>( "width", _windowSize.x );
				w->tryGet<int>( "height", _windowSize.y );
			}

			IniFile::Section *c = _ini->tryGet( "canvas" );
			if( c )
			{
				c->tryGet<glm::mat4>( "mv", _mvCanvas );
				_mvCanvasInv = glm::inverse( _mvCanvas );
			}

			IniFile::Section *a = _ini->tryGet( "app" );
			if( a )
			{
				a->tryGet<bool>( "autosave", _autoSave );
			}
		}
	}

	void Application::save( bool saveScene )
	{
#ifdef __SUPPORT_GUI
		if( _useGui )
			saveIni();
#endif

		if( saveScene )
		{
			if( _sg )
			{
				if( !_sg->save( _sceneName ) )
					std::cerr << "saving scene to \"" << _sceneName << "\" failed" << std::endl;
			}
#ifdef __SUPPORT_GUI
			if( _sgd )
			{
				if( !_sgd->save( _sceneName ) )
					std::cerr << "saving scene UI to \"" << _sceneName << "\" failed" << std::endl;
			}
#endif
		}
	}

	void Application::saveIni()
	{
		if( _ini )
		{
			( *_ini )["window"].set( "posX", _windowPos.x );
			( *_ini )["window"].set( "posY", _windowPos.y );

			( *_ini )["window"].set( "width", _windowSize.x );
			( *_ini )["window"].set( "height", _windowSize.y );

			( *_ini )["canvas"].set( "mv", _mvCanvas );

			( *_ini )["app"].set( "autosave", _autoSave );

			if( !_ini->write() )
				std::cerr << "<error> failed to write window.ini" << std::endl;
		}
	}

	std::string Application::workingDirectory() const
	{
		return std::filesystem::current_path().string();
	}

	std::string Application::executableDirectory() const
	{
#ifdef _WIN32
		char path[MAX_PATH];
		GetModuleFileName( NULL, path, MAX_PATH );
		std::string exe( path );
#else
		char path[PATH_MAX];
		ssize_t count = readlink( "/proc/self/exe", path, PATH_MAX );
		//TODO: UNTESTED!!
		std::string exe( path, ( count > 0 ) ? count : 0 );
#endif
		namespace fs = std::filesystem;

		return fs::path( fs::canonical( fs::absolute( exe ) ) ).parent_path().string();
	}

	void Application::writeToClipboard( const std::string &str )
	{
		try
		{
			clipboardxx::clipboard clipboard;
			clipboard << str.c_str();
		}
		catch( std::exception &e )
		{
			std::cerr << "<error> failed to save to clipboard: \"" << e.what() << "\"" << std::endl;
		}
	}

	std::string Application::readFromClipboard()
	{
		std::string str;

		try
		{
			clipboardxx::clipboard clipboard;
			clipboard >> str;
		}
		catch( std::exception &e )
		{
			std::cerr << "<error> failed to read from clipboard: \"" << e.what() << "\"" << std::endl;
		}

		return str;
	}
#endif
}