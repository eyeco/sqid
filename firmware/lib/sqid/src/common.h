#ifndef _SQID_COMMON
#define _SQID_COMMON

#include "sqid.h"

namespace Sqid
{
	template<typename T>
	inline const T min( const T left, const T right )
	{
		return ( right < left ? right : left );
	}

	template<typename T>
	inline const T max( const T left, const T right )
	{
		return ( left < right ? right : left );
	}

	template<typename T>
	inline T clamp( T value, T minValue, T maxValue )
	{
		return Sqid::min( Sqid::max( value, minValue ), maxValue );
	}

	inline unsigned int nextPo2( unsigned int x )
	{
		unsigned int power = 1;
		while( power < x && power )
			power <<= 1;

		//if( !power )
		//	throw std::runtime_error( "nextpo2 overflow" );

		return power;
	}

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
			delete[] ptr;
			ptr = nullptr;
		}
	}
}

#endif