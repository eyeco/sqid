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

#include <iostream>

#if __GNUC__
#include <cstring>	//memset
#endif

namespace sqid
{
	struct GUID
	{
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];

		GUID()
		{
			memset( this, 0, sizeof( GUID ) );
		}
	};

	SQID_API GUID			SQID_API_CALL guidFromString( const std::string &s );
	SQID_API std::string	SQID_API_CALL guidToString( const GUID &guid );

	SQID_API GUID		SQID_API_CALL randomGUID();

	struct CompareGUID
	{
		bool operator() ( const GUID& lhs, const GUID& rhs ) const
		{
			return ( memcmp( &lhs, &rhs, sizeof( sqid::GUID ) ) < 0 );
		}
	};
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::GUID guid );
SQID_API bool SQID_API_CALL operator == ( const sqid::GUID &lhs, const sqid::GUID &rhs );
SQID_API bool SQID_API_CALL operator != ( const sqid::GUID &lhs, const sqid::GUID &rhs );
