/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <sampleFrame.h>

#include <fileIO/json.h>

namespace sqid
{
	SQID_API const char* SQID_API_CALL interpolationMethodToString( InterpolationMethod method )
	{
		switch( method )
		{
		case IM_NEAREST:
			return "nearest";
		case IM_LINEAR:
			return "linear";
		case IM_CUBIC:
			return "cubic";
		case IM_AREA:
			return "area";
		}
		return "UNKNOWN";
	}

	SQID_API InterpolationMethod SQID_API_CALL interpolationMethodFromString( const char *s )
	{
		if( !s )
			return IM_COUNT;

		for( int i = 0; i < IM_COUNT; i++ )
			if( !_stricmp( s, interpolationMethodToString( (InterpolationMethod) i ) ) )
				return (InterpolationMethod) i;

		return IM_COUNT;
	}

	SQID_API InterpolationMethod SQID_API_CALL interpolationMethodFromString( const std::string &s )
	{
		return interpolationMethodFromString( s.c_str() );
	}

	cv::InterpolationFlags toCVMethod( InterpolationMethod method )
	{
		switch( method )
		{
		case IM_NEAREST:
			return cv::INTER_NEAREST;
		case IM_LINEAR:
			return cv::INTER_LINEAR;
		case IM_CUBIC:
			return cv::INTER_CUBIC;
		case IM_AREA:
			return cv::INTER_AREA;
		}

		std::cerr << "<warning> unable to convert interpolation method: " << interpolationMethodToString( method ) << " (" << (int) method << ")" << std::endl;
		return cv::INTER_LINEAR;
	}

	SampleFrame::SampleFrame() :
		_timeStamp( 0 ),
		_m()
	{
		_m.setTo( 0 );
	}

	SampleFrame::SampleFrame( size_t width, size_t height, uint32_t timeStamp, size_t depth ) :
		_timeStamp( timeStamp ),
		_m( height, width, CV_MAKETYPE( CV_32F, depth ) )
	{
		_m.setTo( 0 );
	}

	SampleFrame::SampleFrame( size_t width, size_t height, const float *f, uint32_t timeStamp, size_t depth ) :
		_timeStamp( timeStamp ),
		_m( height, width, CV_MAKETYPE( CV_32F, depth ) )
	{
		set( f );
	}

	SampleFrame::SampleFrame( const cv::Mat &m, uint32_t timeStamp ) :
		_timeStamp( timeStamp ),
		_m( m )
	{}

	SampleFrame::SampleFrame( const SampleFrame &f ) :
		_timeStamp( f.timeStamp() ),
		_m( f.height(), f.width(), CV_MAKETYPE( CV_32F, f.depth() ) )
	{
		this->set( &f );
	}

	float SampleFrame::minValue() const
	{
		float minValue = std::numeric_limits<float>::max();
		//NOTE: asserts not work for multi-channel
		//for( auto it = _m.begin<float>(); it != _m.end<float>(); ++it )
		//	if( *it < minValue )
		//		minValue = *it;
		const float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			if( *ptr < minValue )
				minValue = *ptr;

		return minValue;
	}

	float SampleFrame::maxValue() const
	{
		float maxValue = std::numeric_limits<float>::lowest();
		//NOTE: asserts not work for multi-channel
		//for( auto it = _m.begin<float>(); it != _m.end<float>(); ++it )
		//	if( *it > maxValue )
		//		maxValue = *it;
		const float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			if( *ptr > maxValue )
				maxValue = *ptr;
		return maxValue;
	}

	void SampleFrame::alloc( size_t width, size_t height, size_t depth )
	{
		_m = cv::Mat( height, width, CV_MAKETYPE( CV_32F, depth ) );
	}

	void SampleFrame::resize( size_t width, size_t height )
	{
		if( width == this->width() && height == this->height() )
			return;

		cv::resize( _m, _m, cv::Size( width, height ), 0.0, 0.0, cv::INTER_CUBIC );
	}

	SampleFrame *SampleFrame::set( float f )
	{
		_m.setTo( f );

		return this;
	}

	SampleFrame *SampleFrame::set( const float *f )
	{
		memcpy( _m.data, f, size() * sizeof( float ) );

		return this;
	}

	SampleFrame *SampleFrame::set( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );
		f->_m.copyTo( _m );

		return this;
	}

	void SampleFrame::setResized( const SampleFrame *f, InterpolationMethod inter, bool warp )
	{
		assert( f && "frame is NULL" );
		assert( inter >= 0 && inter < IM_COUNT && "invalid interpolation method" );

		if( f->width() == this->width() && f->height() == this->height() )
		{
			this->set( f );
			return;
		}

		cv::resize( f->_m, _m, _m.size(), 0.0, 0.0, toCVMethod( inter ) | ( warp ? cv::WARP_FILL_OUTLIERS : 0 ) );
	}

	void SampleFrame::setFlipped( const SampleFrame *f, bool hor, bool ver )
	{
		assert( f && "frame is NULL" );

		if( hor )
		{
			if( ver )
				cv::flip( f->_m, _m, -1 );
			else
				cv::flip( f->_m, _m, 1 );
		}
		else
		{
			if( ver )
				cv::flip( f->_m, _m, 0 );
			else
				set( f );
		}
	}

	void SampleFrame::setTransposed( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );

		cv::transpose( f->_m, _m );
	}

	void SampleFrame::setCropped( const SampleFrame *f, int x, int y, int width, int height )
	{
		assert( f && "frame is NULL" );

		int xs = x;
		int ys = y;
		int w = width;
		int h = height;

		int xd = 0;
		int yd = 0;

		if( x < 0 )
		{
			xd -= x;
			w += x;
			xs = 0;
		}
		if( y < 0 )
		{
			yd -= y;
			h += y;
			ys = 0;
		}

		if( w > f->width() - xs )
			w = f->width() - xs;
		if( h > f->height() - ys )
			h = f->height() - ys;

		_m.create( height, width, CV_MAKETYPE( CV_32F, f->depth() ) );

		if( w > 0 && h > 0 && xs < f->width() && ys < f->height() && xd < width && yd < height )
			f->_m( cv::Rect( xs, ys, w, h ) ).copyTo( _m( cv::Rect( xd, yd, w, h ) ) );
	}

	void SampleFrame::paste( const SampleFrame *f, int x, int y )
	{
		assert( f && "frame is NULL" );
		assert( x + f->width() <= width() );
		assert( y + f->height() <= height() );

		f->_m.copyTo( _m( cv::Rect( x, y, f->width(), f->height() ) ) );
	}

	SampleFrame *SampleFrame::add( float f )
	{
		//NOTE: does not seem to work for multichannel (only adds one channel)
		//_m += f;
		float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			*ptr += f;

		return this;
	}

	SampleFrame *SampleFrame::add( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );

		_m += f->_m;

		return this;
	}

	SampleFrame *SampleFrame::add( const SampleFrame *f, float scale )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );

		_m = _m + f->_m * scale;

		return this;
	}

	SampleFrame *SampleFrame::SampleFrame::sub( float f )
	{
		//NOTE: does not seem to work for multichannel (only adds one channel)
		//_m -= f;
		float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			*ptr -= f;

		return this;
	}

	SampleFrame *SampleFrame::sub( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );
		assert( f->_m.channels() == _m.channels() && "frame depths differ" );

		_m -= f->_m;

		return this;
	}

	SampleFrame *SampleFrame::sub( const SampleFrame *f, float scale )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );
		assert( f->_m.channels() == _m.channels() && "frame depths differ" );

		_m = _m - f->_m * scale;

		return this;
	}

	SampleFrame *SampleFrame::mul( float f )
	{
		//NOTE: does not seem to work for multichannel (only adds one channel)
		//_m *= f;
		float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			*ptr *= f;

		return this;
	}

	SampleFrame *SampleFrame::mul( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );
		assert( f->_m.channels() == _m.channels() && "frame depths differ" );

		cv::multiply( _m, f->_m, _m );

		return this;
	}

	SampleFrame *SampleFrame::div( float f )
	{
		//NOTE: does not seem to work for multichannel (only adds one channel)
		//_m /= f;
		float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			*ptr /= f;

		return this;
	}

	SampleFrame *SampleFrame::div( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );

		_m /= f->_m;

		return this;
	}

	SampleFrame *SampleFrame::pow( float p )
	{
		cv::pow( _m, p, _m );
		return this;
	}

	SampleFrame *SampleFrame::sqrt()
	{
		cv::sqrt( _m, _m );
		return this;
	}

	SampleFrame *SampleFrame::abs()
	{
		_m = cv::abs( _m );
		return this;
	}

	SampleFrame *SampleFrame::log()
	{
		cv::log( _m, _m );
		return this;
	}

	SampleFrame *SampleFrame::exp()
	{
		cv::exp( _m, _m );
		return this;
	}

	SampleFrame *SampleFrame::exp( float base )
	{
		float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			*ptr = std::pow( base, *ptr );

		return this;
	}

	SampleFrame *SampleFrame::exp( const SampleFrame *base )
	{
		assert( base && "frame is NULL" );
		assert( base->_m.size() == _m.size() && "frame sizes differ" );

		float *ptrA = (float*) _m.data;
		float *ptrB = (float*) base->_m.data;
		const float *endA = ptrA + size();
		for( ptrA; ptrA < endA; ptrA++, ptrB++ )
			*ptrA = std::pow( *ptrB, *ptrA );

		return this;
	}

	SampleFrame *SampleFrame::blend( float f, float u )
	{
		//NOTE: does not seem to work for multichannel (only adds one channel)
		//_m = _m * ( 1.0f - u ) + f * u;
		float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			*ptr = *ptr * ( 1.0f - u ) + f * u;

		return this;
	}

	SampleFrame *SampleFrame::blend( const SampleFrame *f, float u )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );

		_m = _m * ( 1.0f - u ) + f->_m * u;

		return this;
	}

	SampleFrame *SampleFrame::threshold( float t, bool binary )
	{
		cv::threshold( _m, _m, t, 1.0f, ( binary ? cv::THRESH_BINARY : cv::THRESH_TOZERO ) );

		return this;
	}

	SampleFrame *SampleFrame::clamp( float minValue, float maxValue )
	{
		cv::min( cv::max( _m, minValue ), maxValue, _m );

		return this;
	}

	SampleFrame *SampleFrame::clamp01()
	{
		return clamp( 0.0f, 1.0f );
	}


	SampleFrame *SampleFrame::logAnd( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );

		float *ptrA = (float*) _m.data;
		float *ptrB = (float*) f->_m.data;
		const float *endA = ptrA + size();
		for( ptrA; ptrA < endA; ptrA++, ptrB++ )
			*ptrA = std::abs( *ptrA * *ptrB ) > std::numeric_limits<float>::epsilon() ? 1.0f : 0.0f;

		return this;
	}

	SampleFrame *SampleFrame::logOr( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );

		float *ptrA = (float*) _m.data;
		float *ptrB = (float*) f->_m.data;
		const float *endA = ptrA + size();
		for( ptrA; ptrA < endA; ptrA++, ptrB++ )
			*ptrA = std::abs( *ptrA ) + std::abs( *ptrB ) > std::numeric_limits<float>::epsilon() ? 1.0f : 0.0f;

		return this;
	}

	SampleFrame *SampleFrame::logXOr( const SampleFrame *f )
	{
		assert( f && "frame is NULL" );
		assert( f->_m.size() == _m.size() && "frame sizes differ" );

		float *ptrA = (float*) _m.data;
		float *ptrB = (float*) f->_m.data;
		const float *endA = ptrA + size();
		for( ptrA; ptrA < endA; ptrA++, ptrB++ )
			*ptrA = ( std::abs( *ptrA ) > std::numeric_limits<float>::epsilon() ? 1 : 0 ) ^ ( std::abs( *ptrB ) > std::numeric_limits<float>::epsilon() ? 1 : 0 );

		return this;
	}

	SampleFrame* SampleFrame::logNot()
	{
		float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			*ptr = std::abs( *ptr ) > std::numeric_limits<float>::epsilon() ? 0.0f : 1.0f;

		return this;
	}

	SampleFrame *SampleFrame::inRange( float minValue, float maxValue )
	{
		float *ptr = (float*) _m.data;
		const float *end = ptr + size();
		for( ptr; ptr < end; ptr++ )
			*ptr = ( *ptr >= minValue && *ptr <= maxValue ? 1.0f : 0.0f );

		return this;
	}

	SampleFrame *SampleFrame::transpose()
	{
		cv::transpose( _m, _m );

		return this;
	}

	SampleFrame *SampleFrame::createFromJSON( const nlohmann::json &j )
	{
		if( j.is_null() )
			return nullptr;

		size_t width = 0;
		size_t height = 0;
		size_t depth = 0;
		uint32_t ts = 0;
		load<size_t>( j, "width", width );
		load<size_t>( j, "height", height );
		load<size_t>( j, "depth", depth );
		load<uint32_t>( j, "ts", ts );

		std::string str;
		load<std::string>( j, "values", str );

		std::stringstream sstr;
		sstr << str;

		SampleFrame *sf = new SampleFrame( width, height, ts, depth );
		float *ptr = sf->values();

		int cntr = 0;
		int size = sf->size();
		std::string sub;
		while( std::getline( sstr, sub, ',' ) )
		{
			if( cntr > size )
			{
				safeDelete( sf );
				throw std::runtime_error( "error reading sf from JSON" );
			}
			ptr[cntr++] = atof( sub.c_str() );
		}

		return sf;
	}

	void SampleFrame::saveToJSON( nlohmann::json &j, const SampleFrame *sf )
	{
		if( !sf )
			return;

		save( j, "width", sf->width() );
		save( j, "height", sf->height() );
		save( j, "depth", sf->depth() );
		save( j, "ts", sf->timeStamp() );

		std::stringstream sstr;
		int size = sf->size();
		for( int i = 0; i < size; i++ )
			sstr << sf->values()[i] << ",";

		save( j, "values", sstr.str() );
	}


	SQID_API bool SQID_API_CALL dimensionsCompatible( const SampleFrame *a, const SampleFrame *b )
	{
		if( a && b )
		{
			return (
				a->width() == b->width() &&
				a->height() == b->height() &&
				a->depth() == b->depth() );
		}

		return true;
	}


	template<>
	SQID_API std::string SQID_API_CALL toString<SampleFrame>( const SampleFrame &sf )
	{
		std::stringstream sstr;

		sstr.precision( 3 );
		sstr << sf.width() << "x" << sf.height();
		if( sf.depth() > 1 )
			sstr << "x" << sf.depth();

		sstr << ", t:" << sf.timeStamp() << " [" << sf.minValue() << ", " << sf.maxValue() << "]";

		return sstr.str();
	}
}


SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::SampleFrame &f )
{
	ostr << "[SampleFrame " << f.width() << " x " << f.height() << " x " << f.depth() << ", t: " << f.timeStamp() << " ]";/* << "\n";

	const float *ptr = f.values();

	auto oldPrec = ostr.precision( 3 );

	if( f.depth() > 1 )
	{
		ostr << "{ ";
		for( int k = 0; k < f.depth(); k++ )
		{
			ostr << ( k ? "\n  " : "" ) << "{ ";
			for( int j = 0; j < f.height(); j++ )
			{
				ostr << ( j ? "\n  " : "" ) << "{ ";
				for( int i = 0; i < f.width(); i++ )
					ostr << ( i ? ", " : "" ) << *( ptr++ );
				ostr << " }";
			}
			ostr << " }";
		}
		ostr << " }";
	}
	else if( f.height() > 1 )
	{
		ostr << "{ ";
		for( int j = 0; j < f.height(); j++ )
		{
			ostr << ( j ? "\n  " : "" ) << "{ ";
			for( int i = 0; i < f.width(); i++ )
				ostr << ( i ? ", " : "" ) << *( ptr++ );
			ostr << " }";
		}
		ostr << " }";
	}
	else
	{
		ostr << "{ ";
		for( int i = 0; i < f.width(); i++ )
			ostr << ( i ? ", " : "" ) << *( ptr++ );
		ostr << " }";
	}

	ostr.precision( oldPrec );
	*/

	return ostr;
}
