/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "serialMsg.h"

#include <config.h>
#include "../compression/compressor.h"
#include "../compression/decompressor.h"

#include <iostream>

namespace sqid
{
	namespace Internal
	{
		const char *msgTypeToString( MsgType msgType )
		{
			switch( msgType )
			{
			case MT_VALUE:
				return "VALUE";
			case MT_ARRAY:
				return "ARRAY";
			case MT_MATRIX:
				return "MATRIX";
			case MT_IMAGE:
				return "IMAGE";
			case MT_PROPERTY:
				return "PROPERTY";
			case MT_FWPROPS_DESC:
				return "FWPROPS_DESC";
			case MT_FWPROPS_STATUS:
				return "FWPROPS_STATUS";
			case MT_FWPROPS_ACK:
				return "FWPROPS_ACK";
			}

			return "UNKNOWN";
		}

		const char *dataTypeToString( MsgFlags flags )
		{
			switch( MF_DATATYPE( flags ) )
			{
			case MF_DATATYPE_BYTE:
				return "UINT8";
			case MF_DATATYPE_INT16:
				return "UINT16";
			case MF_DATATYPE_INT32:
				return "UINT32";
			case MF_DATATYPE_FLOAT:
				return "FLOAT32";
			}

			return "UNKNOWN";
		}

		const char *encodingToString( MsgFlags flags ) 
		{
			switch( MF_ENCODING( flags ) )
			{
			case MF_ENC_UNCOMPRESSED:
				return "RAW";
			case MF_ENC_RLE:
				return "RLE";
			case MF_ENC_LZO:
				return "LZO";
			case MF_ENC_LZ4:
				return "LZ4";
			case MF_ENC_ZLIB:
				return "ZLIB";
			case MF_ENC_ZSTD:
				return "ZSTD";
			}

			return "UNKNOWN";
		}

		size_t getElementSize( MsgFlags type )
		{
			switch( MF_DATATYPE( type ) )
			{
			case MF_DATATYPE_BYTE:
				return 1;
			case MF_DATATYPE_INT16:
				return 2;
			case MF_DATATYPE_INT32:
			case MF_DATATYPE_FLOAT:
				return 4;
			}

			std::cerr << "<error> unknown data type" << std::endl;
			return 0;
		}



		ComMsgParser::ComMsgParser() :
			_desc( "" )
		{}

		ComMsgParser::~ComMsgParser()
		{
			for( auto &it : _decomps )
				safeDelete( it.second );
			_decomps.clear();
		}

#ifdef __COMPRESSION_SUPPORT
		Decompressor *ComMsgParser::getOrCreateDecoder( MsgFlags encoding )
		{
			//should already be passed like this but just to be sure....
			encoding = MF_ENCODING( encoding );

			Decompressor *decomp = nullptr;

			auto it = _decomps.find( encoding );
			if( it == _decomps.end() )
			{
				switch( encoding )
				{
				case MF_ENC_RLE:
					decomp = new DecompressorRLE();
					break;
				case MF_ENC_ZSTD:
#ifdef __COMPRESSION_SUPPORT_ZSTD
					decomp = new DecompressorZStd();
#else
					std::cerr << "<error> compiled without ZStd support" << std::endl;
#endif
					break;
				case MF_ENC_LZO:
				case MF_ENC_LZ4:
				case MF_ENC_ZLIB:
				default:
					std::cerr << "<error> unknown encoding type (or not yet implemented)" << std::endl;
					return nullptr;
				}

				_decomps.insert( std::make_pair( encoding, decomp ) );
			}
			else
				decomp = it->second;

			return decomp;
		}
#endif

		const void *ComMsgParser::getPayload( MsgFlags encoding, const void *data, size_t size, size_t expectedSize )
		{
			if( encoding == MF_ENC_UNCOMPRESSED )
				return data;

#ifdef __COMPRESSION_SUPPORT
			Decompressor *decomp = getOrCreateDecoder( (MsgFlags) encoding );
			if( !decomp )
			{
				std::cerr << "<error> getting decoder failed" << std::endl;
				return nullptr;
			}

			if( _buffer.size() < expectedSize )
				_buffer.resize( expectedSize * 1.5f );
			size_t decodedBytes = decomp->decompress( reinterpret_cast<const unsigned char*>( data ), size, &_buffer[0], _buffer.size(), false );
			if( !decodedBytes )
			{
				std::cerr << "<error> failed decoding" << std::endl;
				return nullptr;
			}

			if( decodedBytes != expectedSize )
				std::cerr << "<warning> decoded block into " << decodedBytes << " (should be " << expectedSize << ")" << std::endl;

			return &_buffer[0];
#else
			std::cerr << "<error> compiled without compression support" << std::endl;
			return nullptr;
#endif
		}

		SampleFrame *ComMsgParser::createFrameFromSingleValue( const ComMsgHdr &hdr, const void *data )
		{
			if( !data )
			{
				std::cerr << "<error> data ptr must not be NULL" << std::endl;
				return nullptr;
			}

			SampleFrame *frm = nullptr;
			const DataHdrSingleValue *dataHdr = reinterpret_cast<const DataHdrSingleValue*>( data );
			const unsigned char *dataPayload = reinterpret_cast<const unsigned char*>( data ) + sizeof( DataHdrSingleValue );

			MsgFlags dt = MF_DATATYPE( dataHdr->flags );
			if( dt != MF_DATATYPE_BYTE && dt != MF_DATATYPE_INT16 && dt != MF_DATATYPE_INT32 && dt != MF_DATATYPE_FLOAT )
			{
				std::cerr << "<error> unknown data type -- header corrupted?" << std::endl;
				return nullptr;
			}

			_desc = "";

			bool isNormalized = dataHdr->flags & MF_NORMALIZED;

			size_t expectedSize = 1 * getElementSize( dt );
			size_t payloadBytes = hdr.dataBytes - sizeof( DataHdrSingleValue );
			MsgFlags encoding = MF_ENCODING( dataHdr->flags );
			const void *values = getPayload( encoding, dataPayload, payloadBytes, expectedSize );
			if( !values )
			{
				std::cerr << "<error> getting data failed" << std::endl;
				return nullptr;
			}

			float ratio = (float) payloadBytes / expectedSize;
			if( encoding == MF_NONE && payloadBytes != expectedSize )
			{
				std::cerr << "<error> expected " << expectedSize << " bytes in uncompressed singleValue frame, but got " << hdr.dataBytes << std::endl;
				return nullptr;
			}

			//TODO: adjust normalization max values here (maybe have to add this to the data header, not sure if a hard-coded value is reasonable here)
			switch( MF_DATATYPE( dataHdr->flags ) )
			{
			case MF_DATATYPE_BYTE:
				frm = createFrame<unsigned char>( 1, 1, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
				break;
			case MF_DATATYPE_INT16:
				frm = createFrame<int16_t>( 1, 1, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
				break;
			case MF_DATATYPE_INT32:
				frm = createFrame<int32_t>( 1, 1, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
				break;
			case MF_DATATYPE_FLOAT:
				frm = createFrame<float>( 1, 1, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
				break;
			default:
				std::cerr << "<error> unknown data type" << std::endl;
				break;
			}

			char tempStr[128];
			sprintf( tempStr, "%s, %s %.2f", dataTypeToString( dataHdr->flags ), encodingToString( dataHdr->flags ), ratio ); //TODO: add compression rate here
			_desc = tempStr;
			
			return frm;
		}

		SampleFrame *ComMsgParser::createFrameFromArray( const ComMsgHdr &hdr, const void *data )
		{
			if( !data )
			{
				std::cerr << "<error> data ptr must not be NULL" << std::endl;
				return nullptr;
			}

			SampleFrame *frm = nullptr;
			const DataHdrArray *dataHdr = reinterpret_cast<const DataHdrArray*>( data );
			const unsigned char *dataPayload = reinterpret_cast<const unsigned char*>( data ) + sizeof( DataHdrArray );

			MsgFlags dt = MF_DATATYPE( dataHdr->flags );
			if( dt != MF_DATATYPE_BYTE && dt != MF_DATATYPE_INT16 && dt != MF_DATATYPE_INT32 && dt != MF_DATATYPE_FLOAT )
			{
				std::cerr << "<error> unknown data type -- header corrupted?" << std::endl;
				return nullptr;
			}

			if( !dataHdr->size )
			{
				std::cerr << "<error> array size is 0 -- header corrupted?" << std::endl;
				return nullptr;
			}

			bool isNormalized = dataHdr->flags & MF_NORMALIZED;

			size_t expectedSize = dataHdr->size * getElementSize( dt );
			size_t payloadBytes = hdr.dataBytes - sizeof( DataHdrArray );
			MsgFlags encoding = MF_ENCODING( dataHdr->flags );
			const void *values = getPayload( encoding, dataPayload, payloadBytes, expectedSize );
			if( !values )
			{
				std::cerr << "<error> getting data failed" << std::endl;
				return nullptr;
			}

			float ratio = (float) payloadBytes / expectedSize;
			if( encoding == MF_NONE && payloadBytes != expectedSize )
			{
				std::cerr << "<error> expected " << expectedSize << " bytes in uncompressed array (" << dataHdr->size << ") frame, but got " << hdr.dataBytes << std::endl;
				return nullptr;
			}

			//TODO: adjust normalization max values here (maybe have to add this to the data header, not sure if a hard-coded value is reasonable here)
			switch( MF_DATATYPE( dataHdr->flags ) )
			{
			case MF_DATATYPE_BYTE:
				frm = createFrame<unsigned char>( dataHdr->size, 1, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
				break;
			case MF_DATATYPE_INT16:
				frm = createFrame<int16_t>( dataHdr->size, 1, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
				break;
			case MF_DATATYPE_INT32:
				frm = createFrame<int32_t>( dataHdr->size, 1, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
				break;
			case MF_DATATYPE_FLOAT:
				frm = createFrame<float>( dataHdr->size, 1, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
				break;
			default:
				std::cerr << "<error> unknown data type" << std::endl;
				break;
			}

			char tempStr[128];
			sprintf( tempStr, "%s, %s %.2f", dataTypeToString( dataHdr->flags ), encodingToString( dataHdr->flags ), ratio ); //TODO: add compression rate here
			_desc = tempStr;

			return frm;
		}

		SampleFrame *ComMsgParser::createFrameFromMatrix( const ComMsgHdr &hdr, const void *data )
		{
			if( !data )
			{
				std::cerr << "<error> data ptr must not be NULL" << std::endl;
				return nullptr;
			}

			SampleFrame *frm = nullptr;
			const DataHdrMatrix *dataHdr = reinterpret_cast<const DataHdrMatrix*>( data );
			const unsigned char *dataPayload = reinterpret_cast<const unsigned char*>( data ) + sizeof( DataHdrMatrix );

			MsgFlags dt = MF_DATATYPE( dataHdr->flags );
			if( dt != MF_DATATYPE_BYTE && dt != MF_DATATYPE_INT16 && dt != MF_DATATYPE_INT32 && dt != MF_DATATYPE_FLOAT )
			{
				std::cerr << "<error> unknown data type -- header corrupted?" << std::endl;
				return nullptr;
			}

			size_t size = dataHdr->width * dataHdr->height;
			if( !size )
			{
				std::cerr << "<error> matrix size is 0 -- header corrupted?" << std::endl;
				return nullptr;
			}

			bool isNormalized = dataHdr->flags & MF_NORMALIZED;

			Decompressor *decomp = nullptr;

			size_t expectedSize = size * getElementSize( dt );
			size_t payloadBytes = hdr.dataBytes - sizeof( DataHdrMatrix );
			MsgFlags encoding = MF_ENCODING( dataHdr->flags );
			const void *values = getPayload( encoding, dataPayload, payloadBytes, expectedSize );
			if( !values )
			{
				std::cerr << "<error> getting data failed" << std::endl;
				return nullptr;
			}

			float ratio = (float) payloadBytes / expectedSize;
			if( encoding == MF_NONE && payloadBytes != expectedSize )
			{
				std::cerr << "<error> expected " << expectedSize << " bytes in uncompressed matrix (" << dataHdr->width << "x" << dataHdr->height << ") frame, but got " << hdr.dataBytes << std::endl;
				return nullptr;
			}

			//TODO: adjust normalization max values here (maybe have to add this to the data header, not sure if a hard-coded value is reasonable here)
			switch( MF_DATATYPE( dataHdr->flags ) )
			{
			case MF_DATATYPE_BYTE:
				frm = createFrame<unsigned char>( dataHdr->width, dataHdr->height, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
				break;
			case MF_DATATYPE_INT16:
				frm = createFrame<int16_t>( dataHdr->width, dataHdr->height, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
				break;
			case MF_DATATYPE_INT32:
				frm = createFrame<int32_t>( dataHdr->width, dataHdr->height, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
				break;
			case MF_DATATYPE_FLOAT:
				frm = createFrame<float>( dataHdr->width, dataHdr->height, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
				break;
			default:
				std::cerr << "<error> unknown data type" << std::endl;
				break;
			}
			
			char tempStr[128];
			sprintf( tempStr, "%s, %s %.2f", dataTypeToString( dataHdr->flags ), encodingToString( dataHdr->flags ), ratio ); //TODO: add compression rate here
			_desc = tempStr;

			return frm;
		}

		SampleFrame* ComMsgParser::createFrameFromImage( const ComMsgHdr& hdr, const void* data )
		{
			if( !data )
			{
				std::cerr << "<error> data ptr must not be NULL" << std::endl;
				return nullptr;
			}

			SampleFrame* frm = nullptr;
			const DataHdrImage* dataHdr = reinterpret_cast<const DataHdrImage*>( data );
			const unsigned char* dataPayload = reinterpret_cast<const unsigned char*>( data ) + sizeof( DataHdrImage );

			MsgFlags dt = MF_DATATYPE( dataHdr->flags );
			if( dt != MF_DATATYPE_BYTE && dt != MF_DATATYPE_INT16 && dt != MF_DATATYPE_INT32 && dt != MF_DATATYPE_FLOAT )
			{
				std::cerr << "<error> unknown data type -- header corrupted?" << std::endl;
				return nullptr;
			}

			size_t size = dataHdr->width * dataHdr->height * dataHdr->depth;
			if( !size )
			{
				std::cerr << "<error> image size is 0 -- header corrupted?" << std::endl;
				return nullptr;
			}

			bool isNormalized = dataHdr->flags & MF_NORMALIZED;

			Decompressor* decomp = nullptr;

			size_t expectedSize = size * getElementSize( dt );
			size_t payloadBytes = hdr.dataBytes - sizeof( DataHdrImage );
			MsgFlags encoding = MF_ENCODING( dataHdr->flags );
			const void* values = getPayload( encoding, dataPayload, payloadBytes, expectedSize );
			if( !values )
			{
				std::cerr << "<error> getting data failed" << std::endl;
				return nullptr;
			}

			float ratio = (float) payloadBytes / expectedSize;
			if( encoding == MF_NONE && payloadBytes != expectedSize )
			{
				std::cerr << "<error> expected " << expectedSize << " bytes in uncompressed image (" << dataHdr->width << "x" << dataHdr->height << "x" << dataHdr->depth << ") frame, but got " << hdr.dataBytes << std::endl;
				return nullptr;
			}

			//TODO: adjust normalization max values here (maybe have to add this to the data header, not sure if a hard-coded value is reasonable here)
			switch( MF_DATATYPE( dataHdr->flags ) )
			{
			case MF_DATATYPE_BYTE:
				frm = createFrame<unsigned char>( dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
				break;
			case MF_DATATYPE_INT16:
				frm = createFrame<int16_t>( dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
				break;
			case MF_DATATYPE_INT32:
				frm = createFrame<int32_t>( dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
				break;
			case MF_DATATYPE_FLOAT:
				frm = createFrame<float>( dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
				break;
			default:
				std::cerr << "<error> unknown data type" << std::endl;
				break;
			}

			char tempStr[128];
			sprintf( tempStr, "%s, %s %.2f", dataTypeToString( dataHdr->flags ), encodingToString( dataHdr->flags ), ratio ); //TODO: add compression rate here
			_desc = tempStr;

			return frm;
		}

		MsgType typeFromFrame( const SampleFrame *frame )
		{
			if( !( frame->width() * frame->height() * frame->depth() ) )
				return MT_COUNT;

			if( frame->depth() == 1 )
			{
				if( frame->height() == 1 )
				{
					if( frame->width() == 1 )
						return MT_VALUE;
					else
						return MT_ARRAY;
				}
				else
					return MT_MATRIX;
			}
			else
				return MT_IMAGE;
		}
	}
}