/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <mutex>
#include <thread>

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#ifdef _WIN32
//#include <conio.h>
//#include <filesystem>
#elif defined __GNUC__
//#include <unistd.h>
//#include <signal.h>
//#include <experimental/filesystem>
#endif

#include <config.h>

#include <app.h>
#include <common.h>

#include <cxxopts.hpp>

#ifdef _WIN32
#include <Windows.h>
#include <Objbase.h>	//COINIT_MULTITHREADED
#endif

using namespace sqid;

namespace sqid
{
	struct StartupConfig
	{
	public:
		StartupConfig() :
#ifdef __SUPPORT_GUI
			useGui( false ),
#endif
			sceneName( "scene" )
		{}

#ifdef __SUPPORT_GUI
		bool useGui;
#endif
		std::string sceneName;

		static bool readPA( int argc, char **argv, StartupConfig &cfg )
		{
			//generic options: 
			cxxopts::Options options( argv[0], " command line options" );
			options
				.positional_help( "[optional args]" )
				.show_positional_help();

			options.add_options()
				//TODO: incorporate version, run RCStamp tool as pre build step and generate include file with version number constants
				//http://www.codeproject.com/KB/dotnet/build_versioning.aspx
				//		( "version,v",										"print version string" )
				( "h,help", "print help" )
#ifdef __SUPPORT_GUI
				( "g,gui", "start with GL visualization" )
#endif
				( "sceneName", "name of scene (which is the scene file minus .JSON extension, relative to working directory)", cxxopts::value<std::string>() )
				;

#ifndef __SUPPORT_GUI
			options.add_options( "hidden" )
				( "g,gui", "start with GL visualization" )
				;
#endif

			//positional options:
			options.parse_positional( { "sceneName" } );

			try
			{
				cxxopts::ParseResult result = options.parse( argc, argv );

				//generic options
				if( result.count( "help" ) )
				{
					std::cout << options.help( {
						""//, 
						//"OSC" 
						} ) << std::endl;

					return false;
				}

				if( result.count( "gui" ) )
				{
#ifdef __SUPPORT_GUI
					cfg.useGui = true;
#else
					throw std::runtime_error( "option g,gui not valid -- binary was built without GUI support" );
#endif
				}

				if( result.count( "sceneName" ) )
					cfg.sceneName = result["sceneName"].as<std::string>();
			}
			catch( std::exception &e )
			{
				std::cerr << e.what() << std::endl;

				std::stringstream sstr;
				sstr << std::endl << "An error occured, processing program options: " << std::endl
					<< e.what() << std::endl;

				std::cout << options.help();

				throw std::runtime_error( sstr.str().c_str() );
			}

			return true;
		}
	};
}


bool cleaned = false;

Application *app = nullptr;




#ifdef _WIN32
bool ctrlHandler( DWORD fdwCtrlType )
{
	switch( fdwCtrlType )
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if( app )
			app->quit();
		return true;
	}

	return false;
}
#elif defined __GNUC__
void sigHandler( int sig )
{
	if( app )
		app->quit();
}
#endif

void cleanup()
{
	if( cleaned )
		return;
	cleaned = true;

	safeDelete( app );
}


void SQID_CDECL onExit()
{
	std::cerr << "on exit called" << std::endl;

	cleanup();
}

void SQID_CDECL onTerminate()
{
	std::cerr << "process terminated" << std::endl;
	abort();
}

int main( int argc, char **argv )
{
	atexit( onExit );
	std::set_terminate( onTerminate );

	srand( time( 0 ) );

	try
	{
#ifdef _WIN32
		if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE) ctrlHandler, true ) )
			throw std::runtime_error( "registering ctrl-handler failed" );
		initCOM( COINIT_MULTITHREADED );
#elif defined __GNUC__
		signal( SIGINT, sigHandler );
#endif

		disableQuickEditMode(); 

		StartupConfig scfg;

		if( StartupConfig::readPA( argc, argv, scfg ) )
		{
			app = Application::create( "sqid" );

#ifdef __SUPPORT_GUI
			app->init( scfg.useGui, scfg.sceneName );
#else
			app->init( false, scfg.sceneName );
#endif

			app->run( argc, argv );

			cleanup();
		}
	}
	catch( std::exception &e )
	{
		std::cerr << "caught exception: " << e.what() << std::endl;

		cleanup();

#ifdef _DEBUG
#ifdef _WIN32
		std::cout << "press ENTER" << std::endl;
		getchar();
#endif
#endif
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
