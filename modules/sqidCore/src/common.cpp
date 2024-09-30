/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <common.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <sstream>
#include <algorithm>
#include <functional>

#ifdef _WIN32
#include <comdef.h>
#elif defined __GNUC__
#include <sys/ioctl.h>
#include <termios.h>
#endif


#ifdef __SUPPORT_GUI

#include "drawing\font.h"

#ifdef _WIN32
#pragma comment( lib, "OpenGL32.lib" )
#pragma comment( lib, "glu32.lib" )
#endif

#endif

#ifdef __COMPRESSION_SUPPORT
#ifdef __COMPRESSION_SUPPORT_LZO
#pragma comment( lib, "miniLZO.lib" )
#endif
#ifdef __COMPRESSION_SUPPORT_QLZ
#pragma comment( lib, "QuickLZ.lib" )
#endif
#ifdef __COMPRESSION_SUPPORT_BZ2
#pragma comment( lib, "libbzip2.lib" )
#endif
#ifdef __COMPRESSION_SUPPORT_ZSTD
#pragma comment( lib, "libzstd_static.lib" )
#endif
#ifdef __COMPRESSION_SUPPORT_ZLIB
#ifdef _DEBUG
#pragma comment( lib, "lz4d.lib" )
#else
#pragma comment( lib, "lz4.lib" )
#endif
#endif
#ifdef __COMPRESSION_SUPPORT_LZ4
#ifdef _DEBUG
#pragma comment( lib, "zlibd.lib" )
#else
#pragma comment( lib, "zlib.lib" )
#endif
#endif
#ifdef __COMPRESSION_SUPPORT_JPEG
#pragma comment( lib, "jpeg.lib" )
#endif
#endif

#ifdef __GNUC__
bool _kbhit()
{
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}
#endif

namespace sqid
{
	SQID_API std::vector<std::string> SQID_API_CALL split( const std::string &s, char delimiter )
	{
		//TODO: 
		// do this properly (this implementation has the following problem: if last substring is "", then it won't be added

		std::string outStr;
		std::stringstream sstr( s );

		std::vector<std::string> v;

		while( std::getline( sstr, outStr, delimiter ) )
			v.push_back( outStr );

		return v;
	}

	SQID_API void SQID_API_CALL trimInPlace( std::string &s )
	{
		trimLeftInPlace( s );
		trimRightInPlace( s );
	}

	SQID_API void SQID_API_CALL trimLeftInPlace( std::string &s )
	{
		if( s.empty() )
			return;

		auto p = s.begin();
		for( p; p != s.end() && isspace( *p ); p++ );

		s.erase( s.begin(), p );
	}

	SQID_API void SQID_API_CALL trimRightInPlace( std::string &s )
	{
		if( s.empty() )
			return;

		auto p = s.end();
		for( p; p != s.begin() && isspace( *--p ););

		if( !isspace( *p ) )
			p++;

		s.erase( p, s.end() );
	}

	SQID_API std::string SQID_API_CALL trim( const std::string &s )
	{
		std::string ret( s );
		trimInPlace( ret );
		return ret;
	}

	SQID_API std::string SQID_API_CALL trimLeft( const std::string &s )
	{
		std::string ret( s );
		trimLeftInPlace( ret );
		return ret;
	}

	SQID_API std::string SQID_API_CALL trimRight( const std::string &s )
	{
		std::string ret( s );
		trimRightInPlace( ret );
		return ret;
	}


	SQID_API void SQID_API_CALL toLowerInPlace( std::string &s )
	{
		std::transform( s.begin(), s.end(), s.begin(), tolower );
	}

	SQID_API void SQID_API_CALL toUpperInPlace( std::string &s )
	{
		std::transform( s.begin(), s.end(), s.begin(), toupper );
	}

	SQID_API std::string SQID_API_CALL toLower( const std::string &s )
	{
		std::string ret( s );
		std::transform( ret.begin(), ret.end(), ret.begin(), tolower );
		return ret;
	}

	SQID_API std::string SQID_API_CALL toUpper( const std::string &s )
	{
		std::string ret( s );
		std::transform( ret.begin(), ret.end(), ret.begin(), toupper );
		return ret;
	}

	SQID_API size_t SQID_API_CALL posOf( const std::string &s, char c, bool ignoreCase, size_t offset )
	{
		if( ignoreCase )
			return s.find( c, offset );

		std::string ls( toLower( s ) );
		char lc = tolower( c );

		return ls.find( lc, offset );
	}

	SQID_API size_t SQID_API_CALL posOf( const std::string &s, const std::string &subStr, bool ignoreCase, size_t offset )
	{
		if( !ignoreCase )
			return s.find( subStr, offset );

		std::string ls( toLower( s ) );
		std::string lss( toLower( subStr ) );

		return ls.find( lss, offset );
	}

	SQID_API std::string SQID_API_CALL replace( const std::string &s, const std::string &oldStr, const std::string &newStr, bool ignoreCase )
	{
		std::string ret = s;
		for( size_t pos = 0; ; pos += newStr.length() )
		{
			// Locate the substring to replace
			pos = posOf( s, oldStr, ignoreCase, pos );
			if( pos == std::string::npos )
				break;

			// Replace by erasing and inserting
			ret.erase( pos, oldStr.length() );
			ret.insert( pos, newStr );
		}
		return ret;
	}

	SQID_API bool SQID_API_CALL contains( const std::string &s, char c, bool ignoreCase )
	{
		return ( posOf( s, c, ignoreCase ) != std::string::npos );
	}

	SQID_API bool SQID_API_CALL contains( const std::string &s, const std::string &subStr, bool ignoreCase )
	{
		return ( posOf( s, subStr, ignoreCase ) != std::string::npos );
	}

	SQID_API bool SQID_API_CALL containsAny( const std::string &s, const std::string &chars, bool ignoreCase )
	{
		for( auto it : chars )
			if( contains( s, it, ignoreCase ) )
				return true;
		return false;
	}

	SQID_API bool SQID_API_CALL startsWith( const std::string &s, const std::string &start, bool ignoreCase )
	{
		if( s.length() < start.length() )
			return false;

		if( ignoreCase )
			return ( s.compare( 0, start.length(), start ) == 0 );

		std::string ls( toLower( s ) );
		std::string lst( toLower( start ) );
		return ( ls.compare( 0, lst.length(), lst ) == 0 );
	}

	SQID_API bool SQID_API_CALL endsWith( const std::string &s, const std::string &end, bool ignoreCase )
	{
		if( s.length() < end.length() )
			return false;

		if( ignoreCase )
			return ( s.compare( s.length() - end.length(), end.length(), end ) == 0 );

		std::string ls( toLower( s ) );
		std::string le( toLower( end ) );
		return ( ls.compare( ls.length() - le.length(), le.length(), le ) == 0 );
	}



	static std::string recordingsDirName( "recordings" );

	SQID_API std::string SQID_API_CALL getRecordingsDirName()
	{
		return recordingsDirName;
	}

	SQID_API void SQID_API_CALL setRecordingsDirName( const std::string &dirName )
	{
		recordingsDirName = std::string( dirName );
	}

#ifdef __COMPRESSION_SUPPORT
	SQID_API const char* SQID_API_CALL compressionAlgorithmToString( CompressionAlgorithm ca )
	{
		switch( ca )
		{
		case CA_NULL:
			return "null";
		case CA_RLE:
			return "RLE";
		case CA_LZO:
			return "MiniLZO";
		case CA_QLZ:
			return "QuickLZ";
		case CA_BZ2:
			return "bzip2";
		case CA_ZSTD:
			return "zStd";
		case CA_ZLIB:
			return "zLib";
		case CA_LZ4:
			return "LZ4";
		case CA_JPEG:
			return "JPEG";
		}

		return "UNKNOWN";
	}

	SQID_API CompressionAlgorithm SQID_API_CALL compressionAlgorithmFromString( const char *s )
	{
		if( !s )
			return CA_COUNT;

		for( int i = 0; i < CA_COUNT; i++ )
			if( !_stricmp( s, compressionAlgorithmToString( (CompressionAlgorithm) i ) ) )
				return (CompressionAlgorithm) i;

		return CA_COUNT;
	}

	SQID_API CompressionAlgorithm SQID_API_CALL compressionAlgorithmFromString( const std::string &s )
	{
		return compressionAlgorithmFromString( s.c_str() );
	}
#endif

	SQID_API const char* SQID_API_CALL interfaceToString( DeviceInterface di )
	{
		switch( di )
		{
		case DI_COM:
			return "COM";
		case DI_RFCOMM:
			return "RFCOMM";
		case DI_OSC:
			return "OSC";
		}

		return "UNKNOWN";
	}

	SQID_API DeviceInterface SQID_API_CALL interfaceFromString( const char *s )
	{
		if( !s )
			return DI_COUNT;

		for( int i = 0; i < DI_COUNT; i++ )
			if( !_stricmp( s, interfaceToString( (DeviceInterface) i ) ) )
				return (DeviceInterface) i;

		return DI_COUNT;
	}

	SQID_API DeviceInterface SQID_API_CALL interfaceFromString( const std::string &s )
	{
		return interfaceFromString( s.c_str() );
	}

	SQID_API std::string SQID_API_CALL formatInterfaceString( DeviceInterface di, unsigned short port )
	{
		char tempStr[128];

		snprintf( tempStr, arraySize( tempStr ), ( di == DI_COM ? "%s%d" : "%s:%d" ), interfaceToString( di ), port );

		return std::string( tempStr );
	}

	SQID_API std::vector<const char*> SQID_API_CALL createDeviceInterfaceComboItems()
	{
		std::vector<const char*> items;
		for( int i = 0; i < DI_COUNT; i++ )
			items.push_back( interfaceToString( (DeviceInterface) i ) );
		return items;
	}

	SQID_API const std::vector<const char*>& SQID_API_CALL getDeviceInterfaceComboItems()
	{
		static std::vector<const char*> comboItems = createDeviceInterfaceComboItems();
		return comboItems;
	}

	SQID_API const char* SQID_API_CALL fileFormatToString( FileFormat format )
	{
		switch( format )
		{
		//case FF_MAT_BIN:
		//	return "MAT BIN";
		//case FF_MAT_ASCII:
		//	return "MAT ASCII";
		case FF_BIN:
			return "BIN";
		case FF_CSV:
			return "CSV";
		}
		return "UNKNOWN";
	}

	SQID_API FileFormat SQID_API_CALL fileFormatFromString( const char *s )
	{
		if( !s )
			return FF_COUNT;

		for( int i = 0; i < FF_COUNT; i++ )
			if( !_stricmp( s, fileFormatToString( (FileFormat) i ) ) )
				return (FileFormat) i;

		return FF_COUNT;
	}

	SQID_API FileFormat SQID_API_CALL fileFormatFromString( const std::string &s )
	{
		return fileFormatFromString( s.c_str() );
	}

	SQID_API const char* SQID_API_CALL fileFormatExtension( FileFormat format )
	{
		switch( format )
		{
		//case FF_MAT_BIN:
		//case FF_MAT_ASCII:
		//	return ".mat";
		case FF_BIN:
			return ".bin";
		case FF_CSV:
			return ".csv";
		}
		return "";
	}

	SQID_API const char* SQID_API_CALL protocolToString( Protocol proto )
	{
		switch( proto )
		{
		case P_UDP:
			return "UDP";
		case P_TCP:
			return "TCP";
		}
		return "UNKNOWN";
	}

	SQID_API Protocol SQID_API_CALL protocolFromString( const char *s )
	{
		if( !s )
			return P_COUNT;

		for( int i = 0; i < P_COUNT; i++ )
			if( !_stricmp( s, protocolToString( (Protocol) i ) ) )
				return (Protocol) i;

		return P_COUNT;
	}

	SQID_API Protocol SQID_API_CALL protocolFromString( const std::string &s )
	{
		return protocolFromString( s.c_str() );
	}




#ifdef _WIN32
	std::string formatWinError( DWORD err )
	{
		if( err == 0 )
			return std::string( "no error" ); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR) &messageBuffer, 0, NULL );

		std::string message( messageBuffer, size );

		//Free the buffer.
		LocalFree( messageBuffer );

		return message;
	}

	std::string formatLastWinError()
	{
		return formatWinError( GetLastError() );
	}

	class COMSingleton
	{
	private:
		DWORD ccm;

	public:
		explicit COMSingleton( DWORD concurrencyModel ) :
			ccm( concurrencyModel )
		{
			HRESULT hr = CoInitializeEx( NULL, ccm );
			if( FAILED( hr ) )
			{
				if( hr == RPC_E_CHANGED_MODE )
					std::cerr << "<error> cannot change COM to different concurrency model" << std::endl;
				else
					std::cerr << "<error> failed to initialize COM with concurrency model " << concurrencyModel << std::endl;
			}
			else 
				std::cout << "successfully initialized COM" << std::endl;
		}

		~COMSingleton()
		{
			CoUninitialize();

			std::cout << "uninitialized COM" << std::endl;
		}

		DWORD getModel() const { return ccm; }
	};

	bool initCOM( DWORD concurrencyModel )
	{
		static COMSingleton singleton( concurrencyModel );
		if( singleton.getModel() != concurrencyModel )
		{
			std::cerr << "<error> COM already initialized with different concurrency model" << std::endl;
			return false;
		}
		return true;
	}
#endif

	void disableQuickEditMode()
	{
#ifdef _WIN32
		HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE );
		unsigned long mode = 0;
		GetConsoleMode( hStdin, &mode );

		mode &= ~ENABLE_QUICK_EDIT_MODE;

		SetConsoleMode( hStdin, mode );
#else
		//TODO
#endif
	}

#ifdef __SUPPORT_GUI
	class FontSingleton
	{
	private:
		Font *_font;

		unsigned int _windowWidth;
		unsigned int _windowHeight;
		
	public:
		FontSingleton() :
			_font( new Font() )
		{}

		~FontSingleton()
		{
			safeDelete( _font );
		}

		bool load( const std::string &fontFile, uint32_t size )
		{
			return _font->load( "resources/courier.ttf", 14 );
		}

		void print( const std::string &str, float x, float y, const glm::vec4 &color, float scale )
		{
			_font->print( str, x, y, _windowWidth, _windowHeight, scale, color );
		}

		void print( const std::string &str, float x, float y, const glm::vec4 &color, float scale, unsigned int canvasWidth, unsigned int canvasHeight )
		{
			_font->print( str, x, y, canvasWidth, canvasHeight, scale, color );
		}

		void updateWindow( unsigned int windowWidth, unsigned int windowHeight )
		{
			_windowWidth = windowWidth;
			_windowHeight = windowHeight;
		}
	};

	FontSingleton &getFont()
	{
		static FontSingleton singleton;
		return singleton;
	}

	SQID_API bool SQID_API_CALL initFont( const std::string &fontFile, uint32_t size, unsigned int windowWidth, unsigned int windowHeight )
	{
		return getFont().load( fontFile, size );
	}

	SQID_API void SQID_API_CALL updateFontWindow( unsigned int windowWidth, int windowHeight )
	{
		getFont().updateWindow( windowWidth, windowHeight );
	}

	SQID_API void SQID_API_CALL printText( const std::string &str, float x, float y, const glm::vec4 &col, float scale )
	{
		getFont().print( str, x, y, col, scale );
	}

	SQID_API void SQID_API_CALL printText( const char *str, float x, float y, const glm::vec4 &col, float scale )
	{
		printText( std::string( str ), x, y, col, scale );
	}

	SQID_API void SQID_API_CALL printText( const std::string &str, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, const glm::vec4 &col, float scale )
	{
		getFont().print( str, x, y, col, scale, canvasWidth, canvasHeight );
	}

	SQID_API void SQID_API_CALL printText( const char *str, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, const glm::vec4 &col, float scale )
	{
		printText( std::string( str ), x, y, canvasWidth, canvasHeight, col, scale );
	}
#endif

	template<>
	SQID_API std::string SQID_API_CALL toString<bool>( const bool &t )
	{
		return std::string( t ? "true" : "false" );
	}
}


SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const glm::vec2 &v )
{
	std::stringstream sstr;
	sstr.precision( 3 );
	sstr << "v2 [" << v.x << ", " << v.y << "]";
	ostr << sstr.str();

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const glm::vec3 &v )
{
	std::stringstream sstr;
	sstr.precision( 3 );
	sstr << "v3 [" << v.x << ", " << v.y << ", " << v.z << "]";
	ostr << sstr.str();

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const glm::vec4 &v )
{
	std::stringstream sstr;
	sstr.precision( 3 );
	sstr << "v4 [" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
	ostr << sstr.str();

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const glm::quat &q )
{
	std::stringstream sstr;
	sstr.precision( 3 );
	sstr << "q [" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << "]";
	ostr << sstr.str();

	return ostr;
}
