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

//#include <opencv2/opencv.hpp>
//#include <nlohmann/json.hpp>

namespace sqid
{
	template<typename T>
	class GenericFrame
	{
	private:
		T _t;
		uint32_t _timeStamp;

	public:
		GenericFrame( const T& t, uint32_t ts ) :
			_t( t ),
			_timeStamp( ts )
		{}

		uint32_t timeStamp() const { return _timeStamp; }
		const T& get() const { return _t; }
	};

	typedef SQID_API GenericFrame<bool> BoolFrame;
	typedef SQID_API GenericFrame<int32_t> Int32Frame;
	typedef SQID_API GenericFrame<float> FloatFrame;
	typedef SQID_API GenericFrame<glm::vec2> Vec2Frame;
	typedef SQID_API GenericFrame<glm::vec3> Vec3Frame;
	typedef SQID_API GenericFrame<glm::vec4> Vec4Frame;
	typedef SQID_API GenericFrame<glm::quat> QuatFrame;
	typedef SQID_API GenericFrame<glm::mat2> Mat2Frame;
	typedef SQID_API GenericFrame<glm::mat3> Mat3Frame;
	typedef SQID_API GenericFrame<glm::mat4> Mat4Frame;


	template<> SQID_API std::string SQID_API_CALL toString<BoolFrame>( const BoolFrame& b );
	template<> SQID_API std::string SQID_API_CALL toString<Int32Frame>( const Int32Frame &i );
	template<> SQID_API std::string SQID_API_CALL toString<FloatFrame>( const FloatFrame &f );
	template<> SQID_API std::string SQID_API_CALL toString<Vec2Frame>( const Vec2Frame &v );
	template<> SQID_API std::string SQID_API_CALL toString<Vec3Frame>( const Vec3Frame &v );
	template<> SQID_API std::string SQID_API_CALL toString<Vec4Frame>( const Vec4Frame &v );
	template<> SQID_API std::string SQID_API_CALL toString<QuatFrame>( const QuatFrame &q );
	template<> SQID_API std::string SQID_API_CALL toString<Mat2Frame>( const Mat2Frame &m );
	template<> SQID_API std::string SQID_API_CALL toString<Mat3Frame>( const Mat3Frame &m );
	template<> SQID_API std::string SQID_API_CALL toString<Mat4Frame>( const Mat4Frame &m );

}

SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream& ostr, const sqid::BoolFrame& b );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Int32Frame &i );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::FloatFrame &f );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Vec2Frame &v );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Vec3Frame &v );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Vec4Frame &v );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::QuatFrame &q );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Mat2Frame &m );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Mat3Frame &m );
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::Mat4Frame &m );
