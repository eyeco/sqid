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

#include <map>
#include <vector>
#include <string>

namespace sqid
{
	class SampleFrame;
	class Decompressor;

	namespace Internal
	{
		static const unsigned int SYNC_BYTES = 2;
		static const unsigned char syncBytes[SYNC_BYTES] = { 0xf7, 0x7f };

		enum ProtocolVersion : unsigned char
		{
			PV_1 = 1,
			PV_2
		};

		enum MsgType : unsigned char
		{
			MT_VALUE,
			MT_ARRAY,
			MT_MATRIX,
			MT_IMAGE,

			//MT_PROPERTY,

			//MT_FWPROPS_DESC,
			//MT_FWPROPS_STATUS,
			//MT_FWPROPS_ACK,

			MT_COUNT
		};

		const char *msgTypeToString( MsgType msgType );

		enum MsgFlags : unsigned short
		{
			MF_NONE = 0,

			MF_NORMALIZED = 0x01 << 0,
			MF_RESERVED_0 = 0x01 << 1,
			MF_RESERVED_1 = 0x01 << 2,

			MF_DATATYPE_MASK = 0x03 << 8,		// bits 8 and 9 specify data type

			MF_DATATYPE_BYTE = 0x00 << 8,
			MF_DATATYPE_INT16 = 0x01 << 8,
			MF_DATATYPE_INT32 = 0x02 << 8,
			MF_DATATYPE_FLOAT = 0x03 << 8,

			MF_ENC_MASK = 0x07 << 10,		// bits 10 thru 12 specify data compression type

			MF_ENC_UNCOMPRESSED = 0x00 << 10,
			MF_ENC_RLE = 0x01 << 10,
			MF_ENC_LZO = 0x02 << 10,
			MF_ENC_LZ4 = 0x03 << 10,
			MF_ENC_ZLIB = 0x04 << 10,
			MF_ENC_ZSTD = 0x05 << 10
		};

		const char *dataTypeToString( MsgFlags flags );
		const char *encodingToString( MsgFlags flags );

#define MF_DATATYPE( a ) ( (MsgFlags)( a & MF_DATATYPE_MASK ) )
#define MF_ENCODING( a ) ( (MsgFlags)( a & MF_ENC_MASK ) )

#pragma pack(push)
#pragma pack(1)
		struct ComMsgHdr
		{
			unsigned char ver;		// protocol version

			MsgType type;			// message type (see enum)

			unsigned char deviceID;
			unsigned char sensorID;

			uint32_t timeStamp;		// milliseconds since startup

			uint16_t dataBytes;		// stride of following data block, in bytes
		};

		struct ComMsgHdrEx
		{
			ComMsgHdr hdr;

			unsigned char chk;		// header checksum
		};

		struct ComMsg
		{
			ComMsgHdrEx hdr;

			void *data;
		};

		struct DataHdrSingleValue
		{
			MsgFlags flags;
		};

		struct DataHdrArray
		{
			MsgFlags flags;
			uint16_t size;
		};

		struct DataHdrMatrix
		{
			MsgFlags flags;
			uint16_t width;
			uint16_t height;
		};

		struct DataHdrImage
		{
			MsgFlags flags;
			uint16_t width;
			uint16_t height;
			uint16_t depth;
		};
#pragma pack(pop)

		inline unsigned char makeChkSum( const ComMsgHdr *hdr )
		{
			unsigned char chk = 0;

			const unsigned char *ptr = reinterpret_cast<const unsigned char*>( hdr );
			for( int i = 0; i < sizeof( ComMsgHdr ); i++ )
				chk += ~( *ptr++ );

			return chk;
		}

		inline bool checkHdr( const ComMsgHdrEx *hdrEx )
		{
			return ( hdrEx->chk == makeChkSum( &hdrEx->hdr ) );
		}

		size_t getElementSize( MsgFlags type );

		class ComMsgParser
		{
		private:
			std::vector<unsigned char> _buffer;
			std::map<MsgFlags,Decompressor*> _decomps;

			std::string _desc;

			Decompressor *getOrCreateDecoder( MsgFlags encoding );
			const void *getPayload( MsgFlags encoding, const void *data, size_t size, size_t expectedSize );

		public:
			ComMsgParser();
			~ComMsgParser();

			std::string getDesc() const { return _desc; }

			SampleFrame *createFrameFromSingleValue( const ComMsgHdr &hdr, const void *data );
			SampleFrame *createFrameFromArray( const ComMsgHdr &hdr, const void *data );
			SampleFrame *createFrameFromMatrix( const ComMsgHdr &hdr, const void *data );
			SampleFrame *createFrameFromImage( const ComMsgHdr &hdr, const void *data );
		};

		MsgType typeFromFrame( const SampleFrame *frame );
	}
}