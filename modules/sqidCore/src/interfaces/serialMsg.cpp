/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "serialMsg.h"

#include <config.h>
#include <interfaces/dataInterface.h>
#include "../compression/compressor.h"
#include "../compression/decompressor.h"

#include <iostream>

namespace sqid
{
	const char* msgTypeToString( MsgType msgType )
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
		}

		return "UNKNOWN";
	}

	const char* dataTypeToString( MsgFlags flags )
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

	const char* encodingToString( MsgFlags flags )
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

	MsgType typeFromFrame( const SampleFrame* frame )
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






	enum Layout
	{
		L_0D,
		L_1D,
		L_2D,
		L_3D,

		L_COUNT
	};

	Layout getLayout( size_t width, size_t height, size_t depth )
	{
		if( !( width * height * depth ) )
			return L_COUNT;

		if( depth == 1 )
		{
			if( height == 1 )
			{
				if( width == 1 )
					return L_0D;
				else
					return L_1D;
			}
			else
				return L_2D;
		}
		else
			return L_3D;
	}



	ComMsgBuilder::ComMsgBuilder() :
		_reservedBytes( 0 ),
		_comMsg( { 0 } )
	{}

	ComMsgBuilder::~ComMsgBuilder()
	{
		safeDeleteArray( _comMsg.data );

#ifdef __COMPRESSION_SUPPORT
		for( auto& it : _comps )
			safeDelete( it.second );
		_comps.clear();
#endif
	}

	void ComMsgBuilder::reserve( size_t bytes )
	{
		if( bytes > _reservedBytes )
		{
			safeDeleteArray( _comMsg.data );
			_comMsg.data = new unsigned char[bytes];
			_reservedBytes = bytes;
		}
	}
		
#ifdef __COMPRESSION_SUPPORT
	Compressor* ComMsgBuilder::getOrCreateEncoder( MsgFlags encoding )
	{
		//should already be passed like this but just to be sure....
		encoding = MF_ENCODING( encoding );

		Compressor* comp = nullptr;

		auto it = _comps.find( encoding );
		if( it == _comps.end() )
		{
			switch( encoding )
			{
			case MF_ENC_RLE:
				comp = new CompressorRLE();
				break;
			case MF_ENC_ZSTD:
#ifdef __COMPRESSION_SUPPORT_ZSTD
				decomp = new CompressorZStd();
#else
				std::cerr << "<error> compiled without ZStd support" << std::endl;
#endif
				break;
			case MF_ENC_LZO:
			case MF_ENC_LZ4:
			case MF_ENC_ZLIB:
				std::cerr << "<error> encoding type not yet implemented" << std::endl;
				return nullptr;
			default:
				std::cerr << "<error> unknown encoding type (or not yet implemented)" << std::endl;
				return nullptr;
			}

			_comps.insert( std::make_pair( encoding, comp ) );
		}
		else
			comp = it->second;

		return comp;
	}
#endif

	bool ComMsgBuilder::getPayload( MsgFlags flags, const SampleFrame* frame, bool normalize, bool clamp )
	{
		size_t size = frame->width() * frame->height() * frame->depth() * getElementSize( flags );

		if( !size )
			return true;

		_payloadBuffer.resize( size );

		bool ret = false;
		switch( MF_DATATYPE( flags ) )
		{
		case MF_DATATYPE_BYTE:
			ret = fromFrame<unsigned char>( frame, reinterpret_cast<unsigned char*>( &_payloadBuffer[0] ), normalize, 0xff, clamp );
			break;
		case MF_DATATYPE_INT16:
			ret = fromFrame<int16_t>( frame, reinterpret_cast<int16_t*>( &_payloadBuffer[0] ), normalize, ( 0x01 << 10 ) - 1, clamp );
			break;
		case MF_DATATYPE_INT32:
			ret = fromFrame<int32_t>( frame, reinterpret_cast<int32_t*>( &_payloadBuffer[0] ), normalize, ( 0x01 << 10 ) - 1, clamp );
			break;
		case MF_DATATYPE_FLOAT:
			ret = fromFrame<float>( frame, reinterpret_cast<float*>( &_payloadBuffer[0] ), normalize, 1.0f, clamp );
			break;
		default:
			std::cerr << "<error> unknown data type" << std::endl;
			break;
		}

		if( !ret )
		{
			std::cerr << "<error> failed converting frame data to specified type" << std::endl;
			_payloadBuffer.clear();
			return false;
		}

		return true;
	}

	const ComMsg* ComMsgBuilder::build( const SampleFrame* frame, unsigned char deviceID, unsigned char sensorID, MsgFlags flags, bool clamp )
	{
		if( !frame )
		{
			std::cerr << "<error> null frame pointer" << std::endl;
			return nullptr;
		}

//		_flags = MF_NONE;
// 		
		MsgType messageType = MT_COUNT;
		switch( getLayout( frame->width(), frame->height(), frame->depth() ) )
		{
		case L_0D:
		{
			messageType = MT_VALUE;
			break;
		}
		case L_1D:
		{
			messageType = MT_ARRAY;
			break;
		}
		case L_2D:
		{
			messageType = MT_MATRIX;
			break;
		}
		case L_3D:
		{
			messageType = MT_IMAGE;
			break;
		}
		default:
		{
			// unknown layout
			std::cerr << "<error> unknown layout" << std::endl;
			return nullptr;
		}
		}

		size_t width = frame->width();
		size_t height = frame->height();
		size_t depth = frame->depth();

		size_t inputDataBytes = width * height * depth * getElementSize( flags );

		bool isNormalized = flags & MF_NORMALIZED;
		if( !getPayload( flags, frame, !isNormalized, clamp ) )
		{
			std::cerr << "<error> failed to get payload" << std::endl;
			return nullptr;
		}

		size_t headerSize = 0;
		size_t payloadSize = _payloadBuffer.size();

		if( messageType == MsgType::MT_VALUE )
		{
			//store header size for later use (data object pointer offset)
			headerSize = sizeof( DataHdrSingleValue );
			//calculate required data array size for a single frame value and set it 
			reserve( headerSize + payloadSize );

			//create value header to set attributes
			DataHdrSingleValue* dataHdrPtr = (DataHdrSingleValue*) _comMsg.data;
			dataHdrPtr->flags = flags;
		}
		else if( messageType == MsgType::MT_ARRAY )
		{
			//store header size for later use (data object pointer offset)
			headerSize = sizeof( DataHdrArray );
			//calculate required data array size for array frame and set it 
			reserve( headerSize + payloadSize );

			//create array header to set attributes
			DataHdrArray* dataArrayHdrPtr = (DataHdrArray*) _comMsg.data;
			dataArrayHdrPtr->flags = flags;
			dataArrayHdrPtr->size = width;
		}
		else if( messageType == MT_MATRIX )
		{
			//store header size for later use (data object pointer offset)
			headerSize = sizeof( DataHdrMatrix );
			//calculate required data array size for a matrix frame and set it 
			reserve( headerSize + payloadSize );

			//create matrix header to set attributes
			DataHdrMatrix* dataMatrixHdrPtr = (DataHdrMatrix*) _comMsg.data;
			dataMatrixHdrPtr->flags = flags;
			dataMatrixHdrPtr->width = width;
			dataMatrixHdrPtr->height = height;
		}
		else if( messageType == MT_IMAGE )
		{
			//store header size for later use (data object pointer offset)
			headerSize = sizeof( DataHdrImage );
			//calculate required data array size for an image frame and set it 
			reserve( headerSize + payloadSize );

			//create image header to set attributes
			DataHdrImage* dataMatrixHdrPtr = (DataHdrImage*) _comMsg.data;
			dataMatrixHdrPtr->flags = flags;
			dataMatrixHdrPtr->width = width;
			dataMatrixHdrPtr->height = height;
			dataMatrixHdrPtr->depth = depth;
		}
		else
		{
			//TODO: print error or something? assert? throw an exception? what's the correct thing to do in arduino code?
			std::cerr << "<error> unknown message type" << std::endl;
			return nullptr;
		}

		//set header type to single value for the receiver to to able to identify the data type
		_comMsg.hdr.hdr.ver = PV_2;
		_comMsg.hdr.hdr.type = messageType;

		_comMsg.hdr.hdr.deviceID = deviceID;
		_comMsg.hdr.hdr.sensorID = sensorID;

		_comMsg.hdr.hdr.timeStamp = frame->timeStamp();

		unsigned char* payloadPtr = (unsigned char*)_comMsg.data + headerSize;

#ifdef __COMPRESSION_SUPPORT
		std::cout << "<error> compression not yet implemented" << std::endl;
		return nullptr;
#else
		memcpy( payloadPtr, &_payloadBuffer[0], payloadSize );
#endif

		_comMsg.hdr.hdr.dataBytes = headerSize + payloadSize;

		//update checksum
		_comMsg.hdr.chk = makeChkSum( &_comMsg.hdr.hdr );

		return &_comMsg;
	}






	ComMsgParser::ComMsgParser() :
		_desc( "" )
	{}

	ComMsgParser::~ComMsgParser()
	{
#ifdef __COMPRESSION_SUPPORT
		for( auto &it : _decomps )
			safeDelete( it.second );
		_decomps.clear();
#endif
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
				std::cerr << "<error> encoding type not yet implemented" << std::endl;
				return nullptr;
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

	const unsigned char *ComMsgParser::getPayload( MsgFlags encoding, const unsigned char *data, size_t size, size_t expectedSize )
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
			frm = toFrame<unsigned char>( 1, 1, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
			break;
		case MF_DATATYPE_INT16:
			frm = toFrame<int16_t>( 1, 1, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_INT32:
			frm = toFrame<int32_t>( 1, 1, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_FLOAT:
			frm = toFrame<float>( 1, 1, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
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
			frm = toFrame<unsigned char>( dataHdr->size, 1, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 0xff );
			break;
		case MF_DATATYPE_INT16:
			frm = toFrame<int16_t>( dataHdr->size, 1, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_INT32:
			frm = toFrame<int32_t>( dataHdr->size, 1, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_FLOAT:
			frm = toFrame<float>( dataHdr->size, 1, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
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
			frm = toFrame<unsigned char>( dataHdr->width, dataHdr->height, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
			break;
		case MF_DATATYPE_INT16:
			frm = toFrame<int16_t>( dataHdr->width, dataHdr->height, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_INT32:
			frm = toFrame<int32_t>( dataHdr->width, dataHdr->height, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_FLOAT:
			frm = toFrame<float>( dataHdr->width, dataHdr->height, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
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
			frm = toFrame<unsigned char>( dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
			break;
		case MF_DATATYPE_INT16:
			frm = toFrame<int16_t>( dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_INT32:
			frm = toFrame<int32_t>( dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_FLOAT:
			frm = toFrame<float>( dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
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
}