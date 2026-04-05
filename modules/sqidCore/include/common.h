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

//#include <cstdio>
#include <string>
//#include <vector>
//#include <fstream>
#include <algorithm>


//#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#include <vld.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/opencv.hpp>

//treat all 4715s as arrors (not all control paths return a value)
#pragma warning (error: 4715)

#ifdef __GNUC__
bool _kbhit();
#endif

namespace sqid
{
	enum ColorSpace
	{
		CS_UNKNOWN,        /* error/unspecified */
		CS_GRAYSCALE,      /* monochrome */
		CS_RGB,            /* red/green/blue as specified by the RGB_RED,
						   RGB_GREEN, RGB_BLUE, and RGB_PIXELSIZE macros */
		CS_YCbCr,          /* Y/Cb/Cr (also known as YUV) */
		CS_CMYK,           /* C/M/Y/K */
		CS_YCCK,           /* Y/Cb/Cr/K */
		CS_RGBX,           /* red/green/blue/x */
		CS_BGR,            /* blue/green/red */
		CS_BGRX,           /* blue/green/red/x */
		CS_XBGR,           /* x/blue/green/red */
		CS_XRGB,           /* x/red/green/blue */
						   /* When out_color_space it set to JCS_EXT_RGBX, JCS_EXT_BGRX, JCS_EXT_XBGR,
						   or JCS_EXT_XRGB during decompression, the X byte is undefined, and in
						   order to ensure the best performance, libjpeg-turbo can set that byte to
						   whatever value it wishes.  Use the following colorspace constants to
						   ensure that the X byte is set to 0xFF, so that it can be interpreted as an
						   opaque alpha channel. */
		CS_RGBA,           /* red/green/blue/alpha */
		CS_BGRA,           /* blue/green/red/alpha */
		CS_ABGR,           /* alpha/blue/green/red */
		CS_ARGB,           /* alpha/red/green/blue */
		CS_RGB565          /* 5-bit red/6-bit green/5-bit blue */
	};

#ifdef __COMPRESSION_SUPPORT
	enum CompressionAlgorithm
	{
		CA_NULL,

		CA_RLE,
		CA_LZO,
		CA_QLZ,
		CA_BZ2,
		CA_ZSTD,
		CA_ZLIB,
		CA_LZ4,

		CA_JPEG,

		CA_COUNT
	};

	SQID_API const char*			SQID_API_CALL compressionAlgorithmToString( CompressionAlgorithm ca );
	SQID_API CompressionAlgorithm	SQID_API_CALL compressionAlgorithmFromString( const char *s );
	SQID_API CompressionAlgorithm	SQID_API_CALL compressionAlgorithmFromString( const std::string &s );
#endif

	enum DeviceInterface : unsigned short
	{
		DI_COM,
		DI_RFCOMM,
		DI_OSC,
		
		DI_COUNT
	};

	SQID_API const char* 		SQID_API_CALL interfaceToString( DeviceInterface di );
	SQID_API DeviceInterface	SQID_API_CALL interfaceFromString( const char *s );
	SQID_API DeviceInterface	SQID_API_CALL interfaceFromString( const std::string &s );

	SQID_API std::string		SQID_API_CALL formatInterfaceString( DeviceInterface di, unsigned short port );

	SQID_API std::vector<const char*>			SQID_API_CALL	createDeviceInterfaceComboItems();
	SQID_API const std::vector<const char*>&	SQID_API_CALL	getDeviceInterfaceComboItems();

	inline uint64_t makeID( DeviceInterface interface, unsigned short portNr, uint8_t deviceID, uint8_t sensorID )
	{
		return ( (uint64_t) portNr << 32 ) | ( (uint64_t) interface << 16 ) | ( ( (uint64_t) deviceID ) << 8 ) | ( (uint64_t) sensorID );
	}

	enum FileFormat
	{
		//FF_MAT_BIN,
		//FF_MAT_ASCII,
		FF_BIN,
		FF_CSV,

		FF_COUNT
	};

	SQID_API const char*	SQID_API_CALL fileFormatToString( FileFormat format );
	SQID_API FileFormat		SQID_API_CALL fileFormatFromString( const char *s );
	SQID_API FileFormat		SQID_API_CALL fileFormatFromString( const std::string &s );

	SQID_API const char*	SQID_API_CALL fileFormatExtension( FileFormat format );

	enum Protocol
	{
		P_UDP,
		P_TCP,

		P_COUNT
	};

	SQID_API const char*	SQID_API_CALL protocolToString( Protocol proto );
	SQID_API Protocol	SQID_API_CALL protocolFromString( const char *s );
	SQID_API Protocol	SQID_API_CALL protocolFromString( const std::string &s );

	template<typename T>
	inline void safeDelete( T* &ptr )
	{
		if( ptr )
		{
			delete ptr;
			ptr = nullptr;
		}
	}

	template<typename T>
	inline void safeDeleteArray( T* &ptr )
	{
		if( ptr )
		{
			delete [] ptr;
			ptr = nullptr;
		}
	}

#ifdef _WIN32
	template<typename T>
	inline void safeRelease( T* &p )
	{
		if( p )
		{
			p->Release();
			p = NULL;
		}
	}
#endif

	//custom min function since std::min produced slow code on vc++
	// http://randomascii.wordpress.com/2013/11/24/stdmin-causing-three-times-slowdown-on-vc/
	template<typename T>
	inline const T min( const T left, const T right )
	{
		return ( right < left ? right : left );
	}

	//custom max function since std::max produced slow code on vc++
	// http://randomascii.wordpress.com/2013/11/24/stdmin-causing-three-times-slowdown-on-vc/
	template<typename T>
	inline const T max( const T left, const T right )
	{
		return ( left < right ? right : left );
	}

	template<typename T>
	inline T clamp( T value, T minValue, T maxValue )
	{
		return sqid::min( sqid::max( value, minValue ), maxValue );
	}

	template<typename T>
	inline T clamp01( T value )
	{
		return sqid::min( sqid::max( value, (T)0 ), (T)1 );
	}

		template<typename T>
	inline float norm( const T &v )
	{
		return sqrt( glm::dot( v, v ) );
	}

	template<typename T>
	inline float norm2( const T &v )
	{
		return glm::dot( v, v );
	}

	inline float sqr( float a )
	{
		return a * a;
	}

	inline long double pi()
	{
		return 3.141592653589793238462643383279502884L;
	}

	inline long double pi2()
	{
		return 2 * 3.141592653589793238462643383279502884L;
	}

	inline float toRad( float deg )
	{
		static float s = pi2() / 360.0f;
		return deg * s;
	}

	inline float toDeg( float rad )
	{
		static float s = 360.0f / pi2();
		return rad * s;
	}

	inline double toRad( double deg )
	{
		static double s = pi2() / 360.0;
		return deg * s;
	}

	inline double toDeg( double rad )
	{
		static double s = 360.0 / pi2();
		return rad * s;
	}

	//--- distance metrics ---
	//NOTE: glm::distance2 is probably faster for glm::vec3 (?)
	inline float distEuclideanSquared( const float *a, const float *b, size_t cnt )
	{
		float val = 0.0f;
		float ret = 0.0f;
		for( int i = 0; i < cnt; i++ )
		{
			val = *( a++ ) - *( b++ );
			ret += val * val;
		}
		return ret;
	}

	//NOTE: glm::distance is probably faster for glm::vec3 (?)
	inline float distEuclidean( const float *a, const float *b, size_t cnt )
	{
		return sqrt( distEuclideanSquared( a, b, cnt ) );
	}

	inline float distManhattan( const float *a, const float *b, size_t cnt )
	{
		float ret = 0.0f;
		for( int i = 0; i < cnt; i++ )
			ret += std::abs( *( a++ ) - *( b++ ) );
		return ret;
	}

	inline float distMaximum( const float *a, const float *b, size_t cnt )
	{
		float ret = 0.0f;
		for( int i = 0; i < cnt; i++ )
			ret = std::max( ret, std::abs( *( a++ ) - *( b++ ) ) );
		return ret;
	}
	//------------------------

	template<typename T>
	T SQID_API_CALL interpolateLinear( const T &a, const T &b, float u )
	{
		return a * ( 1.0f - u ) + b * u;
	}

	template<typename T>
	T SQID_API_CALL interpolateSinusoidal( const T &a, const T &b, float u, float p = 1.0f )
	{
		float x = pow( std::sin( u * pi() * 0.5f ), p );
		return interpolateLinear( a, b, x );
	}

	template<typename T>
	T SQID_API_CALL interpolateCatRomSpline( const T &p_1, const T &p0, const T &p1, const T &p2, float u )
	{
		float u3 = pow( u, 3.0f );
		float u2 = pow( u, 2.0f );

		return 0.5f * (
			p_1 * ( -1 * u3 + 2 * u2 - u ) +
			p0 * ( 3 * u3 - 5 * u2 + 2 ) +
			p1 * ( -3 * u3 + 4 * u2 + u ) +
			p2 * ( 1 * u3 - 1 * u2 - 0 ) );
	}

	template<typename T>
	T SQID_API_CALL multiLinearInterpolation( const std::vector<T> &v, float d )
	{
		assert( v.size() );

		if( v.size() == 1 )
			return v[0];

		d = clamp( d, 0.0f, 1.0f );
		float step = 1.0f / ( v.size() - 1 );

		int i = (int) ( d / step );
		float f = clamp( ( d - i * step ) / step, 0.0f, 1.0f );

		if( i == v.size() - 1 )
			return v[v.size() - 1];

		return v[i] * ( 1.0f - f ) + v[i + 1] * f;
	}

	template <typename T>
	T SQID_API_CALL multiLinearInterpolation( const std::vector<std::pair<T, T> > &kv, float x )
	{
		assert( kv.size() );

		if( kv.size() == 1 )
			return kv[0].second;

		int i = 0;
		for( i; i < kv.size(); i++ )
			if( x <= kv[i].first )
				break;

		if( i > 0 )
		{
			if( i == kv.size() )
				i -= 2;
			else
				i -= 1;
		}

		float x0 = kv[i].first;
		float x1 = kv[i + 1].first;

		if( abs( x1 - x0 ) < 0.0001 )
			return kv[i].second;

		float y0 = kv[i].second;
		float y1 = kv[i + 1].second;

		return y0 + ( x - x0 ) / ( x1 - x0 ) * ( y1 - y0 );
	}

	SQID_API std::vector<std::string> SQID_API_CALL split( const std::string &s, char delimiter );

	SQID_API void SQID_API_CALL trimInPlace( std::string &s );
	SQID_API void SQID_API_CALL trimLeftInPlace( std::string &s );
	SQID_API void SQID_API_CALL trimRightInPlace( std::string &s );

	SQID_API std::string SQID_API_CALL trim( const std::string &s );
	SQID_API std::string SQID_API_CALL trimLeft( const std::string &s );
	SQID_API std::string SQID_API_CALL trimRight( const std::string &s );

	SQID_API void SQID_API_CALL toLowerInPlace( std::string &s );
	SQID_API void SQID_API_CALL toUpperInPlace( std::string &s );

	SQID_API std::string SQID_API_CALL toLower( const std::string &s );
	SQID_API std::string SQID_API_CALL toUpper( const std::string &s );

	SQID_API size_t SQID_API_CALL posOf( const std::string &s, char c, bool ignoreCase = false, size_t offset = 0 );
	SQID_API size_t SQID_API_CALL posOf( const std::string &s, const std::string &subStr, bool ignoreCase = false, size_t offset = 0 );

	SQID_API std::string SQID_API_CALL replace( const std::string &s, const std::string &oldStr, const std::string &newStr, bool ignoreCase = false );

	SQID_API bool SQID_API_CALL contains( const std::string &s, char c, bool ignoreCase = false );
	SQID_API bool SQID_API_CALL contains( const std::string &s, const std::string &subStr, bool ignoreCase = false );

	SQID_API bool SQID_API_CALL containsAny( const std::string &s, const std::string &chars, bool ignoreCase = false );

	SQID_API bool SQID_API_CALL startsWith( const std::string &s, const std::string &start, bool ignoreCase = false );
	SQID_API bool SQID_API_CALL endsWith( const std::string &s, const std::string &end, bool ignoreCase = false );

	template<size_t S>
	void SQID_API_CALL makeDateTimeString( char( &str )[S], time_t time, bool fileSafe = false )
	{
		struct tm *timeinfo;
		timeinfo = localtime( &time );

		snprintf( str, S, ( fileSafe ? "%d-%02d-%02d %02d-%02d-%02d" : "%d-%02d-%02d %02d:%02d:%02d" ),
			timeinfo->tm_year + 1900,
			timeinfo->tm_mon + 1,
			timeinfo->tm_mday,
			timeinfo->tm_hour,
			timeinfo->tm_min,
			timeinfo->tm_sec );
	}

	template<size_t S>
	void SQID_API_CALL makeDateTimeString( char( &str )[S], bool fileSafe = false )
	{
		time_t rawtime;

		::time( &rawtime );

		makeDateTimeString( str, rawtime, fileSafe );
	}

	inline std::string	getDateTimeString( bool fileSafe = false )
	{
		char tempStr[32];
		makeDateTimeString( tempStr, fileSafe );
		return std::string( tempStr );
	}

	inline std::wstring string2wstring( const std::string &str )
	{
		std::wstring wstr( str.begin(), str.end() );
		return wstr;
	}

	inline std::string wstring2string( const std::wstring &wstr )
	{
		std::string str( wstr.begin(), wstr.end() );
		return str;
	}

	template<typename T>
	std::string SQID_API_CALL toString( const T &t )
	{
		std::stringstream sstr;
		sstr << t;

		return sstr.str();
	}

	SQID_API std::string	SQID_API_CALL getRecordingsDirName();
	SQID_API void			SQID_API_CALL setRecordingsDirName( const std::string &dirName );

	inline std::string toRecordingsPath( const std::string &absPath )
	{
		return( getRecordingsDirName() + std::string( "/" ) + absPath );
	}

	template<typename T>
	size_t SQID_API_CALL arraySize( const T& t )
	{
		return sizeof( t ) / sizeof( *t );
	}

	inline unsigned int nextPo2( unsigned int x )
	{
		unsigned int power = 1;
		while( power < x && power )
			power <<= 1;

		if( !power )
			throw std::runtime_error( "nextpo2 overflow" );

		return power;
	}


	inline cv::Point2f glm2cvPf( const glm::vec2 &v )
	{
		return cv::Point2f( v.x, v.y );
	}

	inline cv::Point2d glm2cvPd( const glm::vec2 &v )
	{
		return cv::Point2d( v.x, v.y );
	}

	inline cv::Point3f glm2cvPf( const glm::vec3 &v )
	{
		return cv::Point3f( v.x, v.y, v.z );
	}

	inline cv::Point3d glm2cvPd( const glm::vec3 &v )
	{
		return cv::Point3d( v.x, v.y, v.z );
	}

	inline cv::Vec2f glm2cvVf( const glm::vec2 &v )
	{
		return cv::Vec2f( v.x, v.y );
	}

	inline cv::Vec2d glm2cvVd( const glm::vec2 &v )
	{
		return cv::Vec2d( v.x, v.y );
	}

	inline cv::Vec3f glm2cvVf( const glm::vec3 &v )
	{
		return cv::Vec3f( v.x, v.y, v.z );
	}

	inline cv::Vec3d glm2cvVd( const glm::vec3 &v )
	{
		return cv::Vec3d( v.x, v.y, v.z );
	}

	inline glm::vec2 cv2glm( const cv::Point2f &p )
	{
		return glm::vec2( p.x, p.y );
	}

	inline glm::vec2 cv2glm( const cv::Point2d &p )
	{
		return glm::vec2( p.x, p.y );
	}

	inline glm::vec3 cv2glm( const cv::Point3f &p )
	{
		return glm::vec3( p.x, p.y, p.z );
	}

	inline glm::vec3 cv2glm( const cv::Point3d &p )
	{
		return glm::vec3( p.x, p.y, p.z );
	}

	inline glm::vec2 cv2glm( const cv::Vec2f &v )
	{
		return glm::vec2( v[0], v[1] );
	}

	inline glm::vec2 cv2glm( const cv::Vec2d &v )
	{
		return glm::vec2( v[0], v[1] );
	}

	inline glm::vec3 cv2glm( const cv::Vec3f &v )
	{
		return glm::vec3( v[0], v[1], v[2] );
	}

	inline glm::vec3 cv2glm( const cv::Vec3d &v )
	{
		return glm::vec3( v[0], v[1], v[2] );
	}


	inline const glm::vec3 &red()
	{
		static glm::vec3 r( 1, 0, 0 );
		return r;
	}

	inline const glm::vec3 &green()
	{
		static glm::vec3 g( 0, 1, 0 );
		return g;
	}

	inline const glm::vec3 &blue()
	{
		static glm::vec3 b( 0, 0, 1 );
		return b;
	}

	inline const glm::vec3 &cyan()
	{
		static glm::vec3 c( 0, 1, 1 );
		return c;
	}

	inline const glm::vec3 &magenta()
	{
		static glm::vec3 m( 1, 0, 1 );
		return m;
	}

	inline const glm::vec3 &yellow()
	{
		static glm::vec3 y( 1, 1, 0 );
		return y;
	}

	inline const glm::vec3 &white()
	{
		static glm::vec3 w( 1 );
		return w;
	}

	inline const glm::vec3 &lightgrey()
	{
		static glm::vec3 g( 0.75 );
		return g;
	}

	inline const glm::vec3 &grey()
	{
		static glm::vec3 g( 0.5 );
		return g;
	}

	inline const glm::vec3 &darkgrey()
	{
		static glm::vec3 g( 0.25 );
		return g;
	}

	inline const glm::vec3 &black()
	{
		static glm::vec3 b( 0 );
		return b;
	}

	inline const glm::vec3 &unitX()
	{
		static glm::vec3 x( 1, 0, 0 );
		return x;
	}

	inline const glm::vec3 &unitY()
	{
		static glm::vec3 y( 0, 1, 0 );
		return y;
	}

	inline const glm::vec3 &unitZ()
	{
		static glm::vec3 z( 0, 0, 1 );
		return z;
	}

	inline const glm::vec3 &one()
	{
		static glm::vec3 o( 1 );
		return o;
	}

	inline const glm::vec3 &zero()
	{
		static glm::vec3 z( 0 );
		return z;
	}

	inline const glm::vec3 &unit()
	{
		static glm::vec3 u( sqrt( 3 ) );
		return u;
	}

	inline glm::vec3 cie2rgb( const glm::vec3 &cie )
	{
		static const glm::mat3 matCIE2RGB( 
			3.240479, -0.969256, 0.055648,
			-1.537150, 1.875992, -0.204043,
			-0.498535, 0.041556, 1.057311 );

		return cie * matCIE2RGB;
	}

	inline glm::vec3 rgb2cie( const glm::vec3 &rgb )
	{
		static const glm::mat3 matRGB2CIE( 
			0.412453, 0.212671, 0.019334,
			0.357580, 0.715160, 0.119193,
			0.180423, 0.072169, 0.950227 );

		return rgb * matRGB2CIE;
	}


	//source: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
	// NOTE: hue seemes to be flipped however, changed this here...
	inline glm::vec3 rgb2hsv( const glm::vec3 &rgb )
	{
		static const glm::vec4 K( 0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0 );
		glm::vec4 p = rgb.g < rgb.b ? glm::vec4( rgb.bg, K.wz ) : glm::vec4( rgb.gb, K.xy );
		glm::vec4 q = rgb.r < p.x ? glm::vec4( p.xyw, rgb.r ) : glm::vec4( rgb.r, p.yzx );

		float d = q.x - std::min( q.w, q.y );
		float e = 1.0e-10f;
		return glm::vec3( 1.0f - abs( q.z + ( q.w - q.y ) / ( 6.0 * d + e ) ), d / ( q.x + e ), q.x );
	}

	//source: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
	// NOTE: hue seemes to be flipped however, changed this here...
	inline glm::vec3 hsv2rgb( const glm::vec3 &hsv )
	{
		static const glm::vec4 K( 1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0 );
		glm::vec3 p = abs( glm::fract( one() - hsv.xxx + K.xyz ) * 6.0f - K.www );
		return hsv.z * ( K.xxx * ( 1.0f - hsv.y ) + glm::clamp( p - K.xxx, 0.0f, 1.0f ) * hsv.y );
	}

	inline glm::vec3 rgb2cmy( const glm::vec3 &rgb )
	{
		return one() - rgb;
	}

	inline glm::vec3 cmy2rgb( const glm::vec3 &cmy )
	{
		return one() - cmy;
	}

	//untested -- taken from nvidia samples' color_spaces.cgh, ported to GLSL and simplified
	inline glm::vec4 cmy2cmyk( const glm::vec3 &cmy )
	{
		float k = 1.0;
		k = std::min( k, cmy.x );
		k = std::min( k, cmy.y );
		k = std::min( k, cmy.z );
		if( k >= 1.0f ) //prevent divide-by-zero
			return glm::vec4( 0, 0, 0, 1 );
		glm::vec4 cmyk;
		cmyk.xyz = cmy - glm::vec3( k / ( 1.0f - k ) );
		cmyk.w = k;
		return cmyk;
	}

	//untested -- taken from nvidia samples' color_spaces.cgh, ported to GLSL and simplified
	inline glm::vec3 cmyk2cmy( const glm::vec4 &cmyk )
	{
		return cmyk.xyz * ( 1 - cmyk.w ) + cmyk.w;
	}

	inline glm::vec4 rgb2cmyk( const glm::vec3 &rgb )
	{
		return cmy2cmyk( rgb2cmy( rgb ) );
	}

	inline glm::vec3 cmyk2rgb( const glm::vec4 &cmyk )
	{
		return cmy2rgb( cmyk2cmy( cmyk ) );
	}

	//untested -- taken from nvidia samples' color_spaces.cgh, ported to GLSL and simplified
	inline glm::vec3 rgb2yuv( const glm::vec3 &rgb )
	{
		float y = glm::dot( rgb, glm::vec3( 0.299f, 0.587f, 0.114f ) );
		return glm::vec3( y, ( rgb.z - y ) * 0.565f, ( rgb.x - y ) * 0.713f );
	}

	//untested -- taken from nvidia samples' color_spaces.cgh, ported to GLSL and simplified
	inline glm::vec3 yuv2rgb( const glm::vec3 &yuv )
	{
		return glm::vec3(
			yuv.x + 1.403f * yuv.z,
			yuv.x - 0.344f * yuv.y - 1.403 * yuv.z,
			yuv.x + 1.770f * yuv.y );
	}





	inline glm::vec3 fromHex( unsigned int hex )
	{
		return glm::vec3( 
			( ( hex & 0xff0000 ) >> 16 ) / 255.0f,
			( ( hex & 0x00ff00 ) >> 8 ) / 255.0f,
			( ( hex & 0x0000ff ) ) / 255.0f );
	}

#ifdef __SUPPORT_GUI
	SQID_API bool SQID_API_CALL initFont( const std::string &fontFile, uint32_t size, unsigned int windowWidth, unsigned int windowHeight );
	SQID_API void SQID_API_CALL updateFontWindow( unsigned int windowWidth, int windowHeight );
	SQID_API void SQID_API_CALL printText( const std::string &str, float x, float y, const glm::vec4 &col = glm::vec4( 1 ), float scale = 1.0f );
	SQID_API void SQID_API_CALL printText( const char *str, float x, float y, const glm::vec4 &col = glm::vec4( 1 ), float scale = 1.0f );
	SQID_API void SQID_API_CALL printText( const std::string &str, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, const glm::vec4 &col = glm::vec4( 1 ), float scale = 1.0f );
	SQID_API void SQID_API_CALL printText( const char *str, float x, float y, unsigned int canvasWidth, unsigned int canvasHeight, const glm::vec4 &col = glm::vec4( 1 ), float scale = 1.0f );

	SQID_API bool SQID_API_CALL checkForGLError();

	SQID_API void SQID_API_CALL drawQuad( float left, float bottom, float right, float top );
	SQID_API void SQID_API_CALL drawGraph( const std::vector<float> &values, unsigned int maxValues );//, float bottom, float top )

	SQID_API void SQID_API_CALL drawAxes();
	SQID_API void SQID_API_CALL drawLocator();
	SQID_API void SQID_API_CALL drawGrid();
#endif

	template<class T>
	class HyperRect
	{
		typedef T _ValueType;
		typedef HyperRect<T> _MyType;

	private:
		T _p0;
		T _p1;

	public:
		HyperRect() :
			_p0( 0 ),
			_p1( 0 )
		{}

		HyperRect( const T &p0, const T &p1 ) :
			_p0( p0 ), _p1( p1 )
		{}

		HyperRect( const _MyType &rhs ) :
			_p0( rhs._p0 ),
			_p1( rhs._p1 )
		{}

		T &P0() { return _p0; }
		T &P1() { return _p1; }

		T P0() const { return _p0; }
		T P1() const { return _p1; }

		T Center() const { return ( _p0 + _p1 ) * 0.5f; }

		T Size() const { return _p1 - _p0; }
	};

	typedef HyperRect<glm::vec2> Rect;
	typedef HyperRect<glm::vec3> Box;

	class SQID_API MouseEventHandler
	{
	public:
		virtual ~MouseEventHandler() {}

		virtual bool mouseDown( int button, int mods, bool imGuiHandled ) { return false; }
		virtual bool mouseUp( int button, int mods, bool imGuiHandled ) { return false; }
		virtual void mouseMotion( const glm::vec2 &pos ) {}

		virtual bool scroll( const glm::vec2 &offset, bool imGuiHandled ) { return false; }
	};

	class SQID_API KeyEventHandler
	{
	public:
		virtual ~KeyEventHandler() {}

		virtual bool keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled ) { return false; }
		virtual bool charDown( unsigned char c, bool imGuiHandled ) { return false; }
	};
	
#ifdef _WIN32
	SQID_API std::string SQID_API_CALL formatWinError( DWORD err );
	SQID_API std::string SQID_API_CALL formatLastWinError();

	SQID_API bool SQID_API_CALL initCOM( DWORD concurrencyModel );
#endif

	SQID_API void SQID_API_CALL disableQuickEditMode();

	
	template<> SQID_API std::string SQID_API_CALL toString<bool>( const bool &t );
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const glm::vec2 &v );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const glm::vec3 &v );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const glm::vec4 &v );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const glm::quat &q );
