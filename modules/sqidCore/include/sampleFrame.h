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

#include <common.h>

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

namespace sqid
{
	enum InterpolationMethod
	{
		IM_NEAREST,
		IM_LINEAR,
		IM_CUBIC,
		IM_AREA,

		IM_COUNT
	};

	SQID_API const char*			SQID_API_CALL interpolationMethodToString( InterpolationMethod method );
	SQID_API InterpolationMethod	SQID_API_CALL interpolationMethodFromString( const char *s );
	SQID_API InterpolationMethod	SQID_API_CALL interpolationMethodFromString( const std::string & );

	class SQID_API SampleFrame
	{
	private:
		uint32_t _timeStamp;

		cv::Mat _m;

	public:
		SampleFrame();
		SampleFrame( size_t width, size_t height, uint32_t timeStamp, size_t depth = 1 );
		SampleFrame( size_t width, size_t height, const float *f, uint32_t timeStamp, size_t depth = 1 );
		SampleFrame( const cv::Mat &m, uint32_t timeStamp );
		SampleFrame( const SampleFrame &f );

		size_t width() const { return _m.size().width; }
		size_t height() const { return _m.size().height; }
		size_t depth() const { return _m.channels(); }

		size_t size() const { return _m.size().area() * depth(); }

		uint32_t timeStamp() const { return _timeStamp; }

		float *values() { return (float*) _m.data; }
		const float* values() const { return (const float*) _m.data; }

		cv::Mat &mat() { return _m; }
		const cv::Mat &mat() const { return _m; }

		float minValue() const;
		float maxValue() const;

		void alloc( size_t width, size_t height, size_t depth );
		void resize( size_t width, size_t height );

		SampleFrame *set( float f );
		SampleFrame *set( const float *f );
		SampleFrame *set( const SampleFrame *f );

		void setResized( const SampleFrame *f, InterpolationMethod inter, bool warp = false );
		void setFlipped( const SampleFrame *f, bool hor, bool ver );
		void setCropped( const SampleFrame *f, int x, int y, int width, int height );
		void setTransposed( const SampleFrame *f );

		void paste( const SampleFrame *f, int x, int y );

		SampleFrame *add( float f );
		SampleFrame *add( const SampleFrame *f );
		SampleFrame *add( const SampleFrame *f, float scale );

		SampleFrame *sub( float f );
		SampleFrame *sub( const SampleFrame *f );
		SampleFrame *sub( const SampleFrame *f, float scale );

		SampleFrame *mul( float f );
		SampleFrame *mul( const SampleFrame *f );

		SampleFrame *div( float f );
		SampleFrame *div( const SampleFrame *f );

		SampleFrame *pow( float p );
		SampleFrame *sqrt();

		SampleFrame *abs();

		SampleFrame *log();

		SampleFrame *exp();
		SampleFrame *exp( float base );
		SampleFrame *exp( const SampleFrame *base );

		SampleFrame *blend( float f, float u );
		SampleFrame *blend( const SampleFrame *f, float u );

		SampleFrame *threshold( float t, bool binary = true );

		SampleFrame *clamp( float minValue, float maxValue );
		SampleFrame *clamp01();

		SampleFrame *logAnd( const SampleFrame *f );
		SampleFrame *logOr( const SampleFrame *f );
		SampleFrame *logXOr( const SampleFrame *f );
		SampleFrame *logNot();

		SampleFrame *inRange( float minValue, float maxValue );

		SampleFrame *transpose();

		template<typename UnaryOp>
		SampleFrame *transform( UnaryOp &op ) 
		{
			float *ptr = (float*) _m.data;
			const float *end = ptr + size();
			for( ptr; ptr < end; ptr++ )
				*ptr = op( *ptr );

			return this;
		}

		static SampleFrame *createFromJSON( const nlohmann::json &j );
		static void saveToJSON( nlohmann::json &j, const SampleFrame *sf );
	};

	SQID_API bool SQID_API_CALL dimensionsCompatible( const SampleFrame *a, const SampleFrame *b );

	template<typename T>
	inline SampleFrame *createFrame( unsigned int width, unsigned int height, unsigned int depth, const T *values, uint32_t ts, bool normalize = false, T maxValue = std::numeric_limits<T>::max(), bool clamp = false )
	{
		if( !values )
			return nullptr;

		SampleFrame *frame = new SampleFrame( width, height, ts, depth );

		int size = width * height * depth;

		if( normalize )
		{
			float s = ( normalize ? 1.0f / maxValue : 1.0f );
			if( clamp )
			{
				for( int i = 0; i < size; i++ )
					frame->values()[i] = sqid::clamp<float>( values[i] * s, 0.0f, 1.0f );
			}
			else
			{
				for( int i = 0; i < size; i++ )
					frame->values()[i] = values[i] * s;
			}
		}
		else
			for( int i = 0; i < size; i++ )
				frame->values()[i] = values[i];

		return frame;
	}

	struct SampleFrameContainer
	{
		short deviceID;
		short sensorID;

		std::string message;

		SampleFrame *frame;

		SampleFrameContainer( unsigned char deviceID, unsigned char sensorID, SampleFrame *frame ) :
			deviceID( deviceID ),
			sensorID( sensorID ),
			message( "" ),
			frame( frame )
		{}

		SampleFrameContainer( const std::string &message, SampleFrame *frame ) :
			deviceID( 0xff ),
			sensorID( 0xff ),
			message( message ),
			frame( frame )
		{}

		//static std::string makeMessage( unsigned char deviceID, unsigned char sensorID )
		//{
		//	char tempStr[64];
		//	sprintf( tempStr, "did:0x%02x sid:0x%02x", (int) deviceID, (int) sensorID );

		//	return std::string( tempStr );
		//}
	};

	template<> SQID_API std::string SQID_API_CALL toString<SampleFrame>( const SampleFrame &sf );
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::SampleFrame &f );