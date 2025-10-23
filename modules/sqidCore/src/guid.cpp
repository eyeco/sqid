/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <guid.h>

#if __GNUC__
#include <stdio.h>	//sscanf_s
#endif

namespace sqid
{
	SQID_API GUID SQID_API_CALL guidFromString( const std::string &s )
	{
		GUID guid;

		unsigned short dummyUShort = 0;
		unsigned long long int dummyULongLong = 0;

		int read = sscanf( s.c_str(), "%lx-%hx-%hx-%hx-%llx",
			&guid.Data1,
			&guid.Data2,
			&guid.Data3,
			&dummyUShort,
			&dummyULongLong );

		guid.Data4[0] = ( dummyUShort >> 8 ) & 0xff;
		guid.Data4[1] = ( dummyUShort >> 0 ) & 0xff;

		guid.Data4[2] = ( dummyULongLong >> 40 ) & 0xff;
		guid.Data4[3] = ( dummyULongLong >> 32 ) & 0xff;
		guid.Data4[4] = ( dummyULongLong >> 24 ) & 0xff;
		guid.Data4[5] = ( dummyULongLong >> 16 ) & 0xff;
		guid.Data4[6] = ( dummyULongLong >> 8 ) & 0xff;
		guid.Data4[7] = ( dummyULongLong >> 0 ) & 0xff;

		return guid;
	}

	SQID_API std::string SQID_API_CALL guidToString( const GUID &guid )
	{
		char tempStr[64];
		sprintf( tempStr, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1],
			guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] );

		return std::string( tempStr );
	}

	SQID_API GUID SQID_API_CALL randomGUID()
	{
		GUID guid;
		short *ptr = reinterpret_cast<short*>( &guid );

		for( int i = 0; i < ( sizeof( GUID ) / sizeof( short ) ); i++ )
			*( ptr++ ) = rand();

		return guid;
	}
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::GUID guid )
{
	ostr << sqid::guidToString( guid ).c_str();

	return ostr;
}

SQID_API bool SQID_API_CALL operator == ( const sqid::GUID &lhs, const sqid::GUID &rhs )
{
	return ( memcmp( &lhs, &rhs, sizeof( sqid::GUID ) ) == 0 );
}

SQID_API bool SQID_API_CALL operator != ( const sqid::GUID &lhs, const sqid::GUID &rhs )
{
	return !( lhs == rhs );
}