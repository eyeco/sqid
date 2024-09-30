/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <blobFrame.h>

namespace sqid
{
	template<>
	std::string toString<BlobFrame>( const BlobFrame &bf )
	{
		std::stringstream sstr;

		sstr << bf.size() << ", t:" << bf.timeStamp();

		return sstr.str();
	}

#ifdef __COMPRESSION_SUPPORT
	template<>
	std::string toString<CompressedSampleFrame>( const CompressedSampleFrame &csf )
	{
		std::stringstream sstr;

		sstr << csf.bytes() << " (" << compressionAlgorithmToString( csf.algorithm() ) << "), t:" << csf.timeStamp();

		return sstr.str();
	}
#endif
}


SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::BlobFrame &f )
{
	ostr << "[BlobFrame " << f.size() << "]";

	return ostr;
}

#ifdef __COMPRESSION_SUPPORT
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::CompressedSampleFrame &f )
{
	ostr << "[CompressedSampleFrame " << f.bytes() << " (" << sqid::compressionAlgorithmToString( f.algorithm() ) << ")]";

	return ostr;
}
#endif
