/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <processing/pin.h>
#include <processing/op.h>

#include <blobFrame.h>
#include <sampleFrame.h>

//define container for use across modules
#define DEFINE_PUBLIC_CONTAINER( _DATA_TYPE, _NAME, _SHORT_NAME, _GUID ) \
	template<> const GUID SQID_API DataContainer<_DATA_TYPE>::typeGUID = guidFromString( _GUID ); \
	template<> const std::string SQID_API DataContainer<_DATA_TYPE>::typeName( _NAME ); \
	template<> const std::string SQID_API DataContainer<_DATA_TYPE>::typeShortName( _SHORT_NAME );

namespace sqid
{
	DEFINE_PUBLIC_CONTAINER( BoolFrame, "bool", "b", "501B227B-99FA-4A84-AA8F-4192D0478B03" )
	DEFINE_PUBLIC_CONTAINER( Int32Frame, "int32", "i32", "1C8DEA98-779F-427C-885C-F9044B717A38" )
	DEFINE_PUBLIC_CONTAINER( FloatFrame, "float", "f", "78DD57E2-2D62-45CF-8C35-28F09B91CB80" )
	DEFINE_PUBLIC_CONTAINER( Vec2Frame, "vec2", "v2", "510E91D4-1E9C-4DF5-94B6-D22BF852240D" )
	DEFINE_PUBLIC_CONTAINER( Vec3Frame, "vec3", "v3", "4EF47260-74F7-472C-ABB4-D4323C887F8B" )
	DEFINE_PUBLIC_CONTAINER( Vec4Frame, "vec4", "v4", "7BEC21C1-8C94-4AAC-9DAA-0CD6AC5BC6F6" )
	DEFINE_PUBLIC_CONTAINER( QuatFrame, "quat", "q", "763F2635-E3F4-4D76-9F0B-5858B3B09B6F" )
	DEFINE_PUBLIC_CONTAINER( Mat2Frame, "mat2", "m2", "B9E3C7E2-8513-408A-917E-5B6BDCC83AD4" )
	DEFINE_PUBLIC_CONTAINER( Mat3Frame, "mat3", "m3", "09960A14-9EC7-469B-B488-700887F86F41" )
	DEFINE_PUBLIC_CONTAINER( Mat4Frame, "mat4", "m4", "6236ED16-E1A1-4193-BADA-A8CE66360DCC" )
	DEFINE_PUBLIC_CONTAINER( SampleFrame, "SampleFrame", "sf", "62111F9D-3696-4F1D-8AF5-F729C1581788" )
	


	//DEFINE_CONTAINER( BlobFrame, "BlobFrame", "bf", "AB69C161-0F5D-49E2-85C8-D9BE72D1DC54" )
#ifdef __COMPRESSION_SUPPORT
	//DEFINE_CONTAINER( CompressedSampleFrame, "CompressedSampleFrame", "csf", "E34D9BB2-96B1-49B2-BD74-70101C2E9AEF" )
#endif

	//DEFINE_CONTAINER( PointCloud, "PointCloud", "pc", "18C68926-46D6-4067-AA78-FCBEC7C8452D" )

	//NOTE: have to hide usage of class Op here in cpp to resolve cyclic template dependency OutletPin <-> Op
	bool Pin::isOpEnabled() const { return _op->getEnabled(); }
}
