/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "sqidMidi.h"
#include "midiOps.h"

#include <processing/op.h>
#include <processing/opFactory.h>

#include <iostream>

#ifdef _WIN32 
#include <Windows.h>
#endif

#ifdef _WIN32 
void on_load()
#else
__attribute__( ( constructor ) )
void on_load()
#endif
{
	std::cout << "on_load" << std::endl;
}

#ifdef _WIN32 
void on_unload()
#else
__attribute__( ( constructor ) )
void on_load()
#endif
{
	std::cout << "on_unload" << std::endl;
}

#ifdef _WIN32 
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved ) {
	switch( fdwReason )
	{
	case DLL_PROCESS_ATTACH:
		try
		{
			on_load();
		}
		catch( std::runtime_error &e )
		{
			std::cerr << "<error> dll loading failed: " << e.what() << std::endl;
			return FALSE;
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		on_unload();
		break;
	}
	return TRUE;
}
#endif

EXTERN_DLL_EXPORT sqid::PluginInterface* createPlugin()
{
	return new sqid::Plugins::SqidMidi();
}

EXTERN_DLL_EXPORT void deletePlugin( sqid::PluginInterface *pi )
{
	return delete pi;
}

namespace sqid
{
	namespace Plugins
	{
		SqidMidi::SqidMidi()
		{
			std::cout << "SqidMidi::ctor" << std::endl;
		}

		SqidMidi::~SqidMidi()
		{
			std::cout << "SqidMidi::dtor" << std::endl;
		}

		void SqidMidi::printBuildInfo() const
		{
			//TODO: automatic app versioning?
			std::cout << getName() << " version " << __PLUGIN_VERSION_MAJOR << "." << __PLUGIN_VERSION_MINOR << "." << __PLUGIN_VERSION_PATCH << std::endl;
			std::cout << "built for sqid version " << __VERSION_MAJOR << "." << __VERSION_MINOR << "." << __VERSION_PATCH << std::endl;

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
		}

		bool SqidMidi::init()
		{
			std::cout << "SqidMidi::init" << std::endl;

			return true;
		}

		void SqidMidi::uninit()
		{
			std::cout << "SqidMidi::uninit" << std::endl;
		}

		bool SqidMidi::registerOps()
		{
			std::cout << "SqidMidi::registerOps" << std::endl;

			REGISTER_OP_TYPE( sqid::Plugins::MIDIIn );
			REGISTER_OP_TYPE( sqid::Plugins::MIDIOut );

			return true;
		}

		void SqidMidi::enumerateCaps()
		{
			std::cout << "SqidMidi::enumerateCaps" << std::endl;
		}
	}
}