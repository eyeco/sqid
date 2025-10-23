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

#ifdef SQID_DLL
#define SQID_EXP __declspec(dllexport)
#define SQID_IMP __declspec(dllimport)
#define SQID_API_CALL __stdcall
#else
#define	SQID_EXP
#define SQID_IMP
#define SQID_API_CALL
#endif

#ifdef SQID_EXPORTS
#define SQID_API SQID_EXP
#pragma warning( disable: 4251 )
#pragma warning( disable: 4661 )
#define EXPIMP_TEMPLATE
#else
#define SQID_API SQID_IMP
#pragma warning( disable: 4251 )
#define EXPIMP_TEMPLATE extern
#endif


#pragma warning( disable : 4018 )	//disable signed/unsigned mismatch warning
#pragma warning( disable : 4244 )	//disable type conversion warnings
#pragma warning( disable : 4267 )	//disable size_t type conversion warnings

#define STR(x)  STR2(x)
#define STR2(x) #x

#define __VERSION_MAJOR		0
#define __VERSION_MINOR		1
#define __VERSION_PATCH		2

#define __VERSION_STRING	STR( __VERSION_MAJOR ) "." STR( __VERSION_MINOR ) "." STR( __VERSION_PATCH )

#ifdef _WIN32
#  define SQID_CDECL __cdecl
#else
#  define SQID_CDECL //__attribute__ ((__cdecl__))
#endif

#ifdef __GNUC__
#  define _stricmp strcasecmp
typedef unsigned int DWORD;
#endif

#define __FFTW_SUPPORT
//#define __COMPRESSION_SUPPORT

#define __RFCOMM_SUPPORT

#define __DRAW_SPLINE_CONNECTORS
#define __BUFFER_INPUT_PINS

#define __SUPPORT_GUI
//#define __QUIT_WITH_ESCAPE

//NOTE: removed call of makeHeaders.bat from Pre-Build Events since it would trigger a re-build of all 
// depending source files, even if nothing changed in the code. Furthermore, there apparently is no way 
// of calling it only if __STATIC_SHADERS is defined. therefore: call it manually whenever you update
// shader code (find it in sqid/resources/shaders/makeHeaders.bat). 
#define __STATIC_SHADERS

#ifdef __COMPRESSION_SUPPORT
#  define __COMPRESSION_SUPPORT_LZO
#  define __COMPRESSION_SUPPORT_QLZ
#  define __COMPRESSION_SUPPORT_BZ2
//#  define __COMPRESSION_SUPPORT_ZSTD
#  define __COMPRESSION_SUPPORT_ZLIB
#  define __COMPRESSION_SUPPORT_LZ4
#  define __COMPRESSION_SUPPORT_JPEG
#endif

