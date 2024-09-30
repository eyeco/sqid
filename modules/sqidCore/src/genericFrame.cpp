/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <genericFrame.h>

//#include <fileIO/json.h>

namespace sqid
{
	template<>
	SQID_API std::string SQID_API_CALL toString<BoolFrame>( const BoolFrame& b )
	{
		std::stringstream sstr;

		sstr << toString( b.get() ) << ", t: " << b.timeStamp();

		return sstr.str();
	}

		template<>
		SQID_API std::string SQID_API_CALL toString<Int32Frame>( const Int32Frame &i )
		{
			std::stringstream sstr;
	
			sstr << i.get() << ", t: " << i.timeStamp();
	
			return sstr.str();
		}
	
		template<>
		SQID_API std::string SQID_API_CALL toString<FloatFrame>( const FloatFrame &f )
		{
			std::stringstream sstr;
	
			sstr.precision( 3 );
			sstr << f.get() << ", t: " << f.timeStamp();
	
			return sstr.str();
		}
	
		template<>
		SQID_API std::string SQID_API_CALL toString<Vec2Frame>( const Vec2Frame &v )
		{
			std::stringstream sstr;
	
			sstr.precision( 3 );
			sstr << v.get().x << "/" << v.get().y << ", t: " << v.timeStamp();
	
			return sstr.str();
		}
	
		template<>
		SQID_API std::string SQID_API_CALL toString<Vec3Frame>( const Vec3Frame &v )
		{
			std::stringstream sstr;
	
			sstr.precision( 3 );
			sstr << v.get().x << "/" << v.get().y << "/" << v.get().z << ", t: " << v.timeStamp();
	
			return sstr.str();
		}
	
		template<>
		SQID_API std::string SQID_API_CALL toString<Vec4Frame>( const Vec4Frame &v )
		{
			std::stringstream sstr;
	
			sstr.precision( 3 );
			sstr << v.get().x << "/" << v.get().y << "/" << v.get().z << "/" << v.get().w << ", t: " << v.timeStamp();
	
			return sstr.str();
		}
	
		template<>
		SQID_API std::string SQID_API_CALL toString<QuatFrame>( const QuatFrame &q )
		{
			std::stringstream sstr;
	
			sstr.precision( 3 );
			sstr << q.get().x << "/" << q.get().y << "/" << q.get().z << "/" << q.get().w << ", t: " << q.timeStamp();
	
			return sstr.str();
		}
	
		template<>
		SQID_API std::string SQID_API_CALL toString<Mat2Frame>( const Mat2Frame &m )
		{
			std::stringstream sstr;
	
			sstr << "m2x2, t:" << m.timeStamp();
	
			return sstr.str();
		}
	
		template<>
		SQID_API std::string SQID_API_CALL toString<Mat3Frame>( const Mat3Frame &m )
		{
			std::stringstream sstr;
	
			sstr << "m3x3, t:" << m.timeStamp();
	
			return sstr.str();
		}
	
		template<>
		SQID_API std::string SQID_API_CALL toString<Mat4Frame>( const Mat4Frame &m )
		{
			std::stringstream sstr;
	
			sstr << "m4x4, t:" << m.timeStamp();
	
			return sstr.str();
		}
}


SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream& ostr, const sqid::BoolFrame& b )
{
	ostr << "[bool " << b.timeStamp() << "]";

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Int32Frame &i )
{
	ostr << "[int32 " << i.timeStamp() << "]";

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::FloatFrame &f )
{
	ostr << "[float " << f.timeStamp() << "]";

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Vec2Frame &v )
{
	ostr << "[vec2 " << v.timeStamp() << "]";

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Vec3Frame &v )
{
	ostr << "[vec3 " << v.timeStamp() << "]";

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Vec4Frame &v )
{
	ostr << "[vec4 " << v.timeStamp() << "]";

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::QuatFrame &q )
{
	ostr << "[quaterninon " << q.timeStamp() << "]";

	return ostr;
}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Mat2Frame &m )
{
	ostr << "[matrix2x2 " << m.timeStamp() << "]";

	return ostr;
}


SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Mat3Frame &m )
{
	ostr << "[matrix3x3 " << m.timeStamp() << "]";

	return ostr;
}


SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Mat4Frame &m )
{
	ostr << "[matrix4x4 " << m.timeStamp() << "]";

	return ostr;
}
