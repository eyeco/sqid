#include "comMsgParser.h"

#include "sampleFrame.h"

#include "stdio.h"

namespace sqid
{
	ComMsgParser::ComMsgParser()
	{}

	ComMsgParser::~ComMsgParser()
	{}

	const unsigned char *ComMsgParser::getPayload( MsgFlags encoding, const unsigned char *data, size_t size, size_t expectedSize )
	{
		if( encoding == MF_ENC_UNCOMPRESSED )
			return data;

		//std::cerr << "<error> compiled without compression support" << std::endl;
		return nullptr;
	}

	SampleFrame *ComMsgParser::createFrameFromSingleValue( const ComMsgHdr &hdr, const void *data, DataType outType )
	{
		if( !data )
		{
			//std::cerr << "<error> data ptr must not be NULL" << std::endl;
			return nullptr;
		}

		SampleFrame *frm = nullptr;
		const DataHdrSingleValue *dataHdr = reinterpret_cast<const DataHdrSingleValue*>( data );
		const unsigned char *dataPayload = reinterpret_cast<const unsigned char*>( data ) + sizeof( DataHdrSingleValue );

		MsgFlags dt = MF_DATATYPE( dataHdr->flags );
		if( dt != MF_DATATYPE_BYTE && dt != MF_DATATYPE_INT16 && dt != MF_DATATYPE_INT32 && dt != MF_DATATYPE_FLOAT )
		{
			//std::cerr << "<error> unknown data type -- header corrupted?" << std::endl;
			return nullptr;
		}

		_desc[0] = 0;

		bool isNormalized = dataHdr->flags & MF_NORMALIZED;

		size_t expectedSize = 1 * getElementSize( dt );
		size_t payloadBytes = hdr.dataBytes - sizeof( DataHdrSingleValue );
		MsgFlags encoding = MF_ENCODING( dataHdr->flags );
		const void *values = getPayload( encoding, dataPayload, payloadBytes, expectedSize );
		if( !values )
		{
			//std::cerr << "<error> getting data failed" << std::endl;
			return nullptr;
		}

		float ratio = (float) payloadBytes / expectedSize;
		if( encoding == MF_NONE && payloadBytes != expectedSize )
		{
			//std::cerr << "<error> expected " << expectedSize << " bytes in uncompressed singleValue frame, but got " << hdr.dataBytes << std::endl;
			return nullptr;
		}

		//TODO: adjust normalization max values here (maybe have to add this to the data header, not sure if a hard-coded value is reasonable here)
		switch( MF_DATATYPE( dataHdr->flags ) )
		{
		case MF_DATATYPE_BYTE:
			frm = toFrame<unsigned char>( outType, 1, 1, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
			break;
		case MF_DATATYPE_INT16:
			frm = toFrame<int16_t>( outType, 1, 1, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_INT32:
			frm = toFrame<int32_t>( outType, 1, 1, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_FLOAT:
			frm = toFrame<float>( outType, 1, 1, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
			break;
		default:
			//std::cerr << "<error> unknown data type" << std::endl;
			break;
		}

		sprintf( _desc, "%s, %s %.2f", dataTypeToString( dataHdr->flags ), encodingToString( dataHdr->flags ), ratio ); //TODO: add compression rate here
			
		return frm;
	}

	SampleFrame *ComMsgParser::createFrameFromArray( const ComMsgHdr &hdr, const void *data, DataType outType )
	{
		if( !data )
		{
			//std::cerr << "<error> data ptr must not be NULL" << std::endl;
			return nullptr;
		}

		SampleFrame *frm = nullptr;
		const DataHdrArray *dataHdr = reinterpret_cast<const DataHdrArray*>( data );
		const unsigned char *dataPayload = reinterpret_cast<const unsigned char*>( data ) + sizeof( DataHdrArray );

		MsgFlags dt = MF_DATATYPE( dataHdr->flags );
		if( dt != MF_DATATYPE_BYTE && dt != MF_DATATYPE_INT16 && dt != MF_DATATYPE_INT32 && dt != MF_DATATYPE_FLOAT )
		{
			//std::cerr << "<error> unknown data type -- header corrupted?" << std::endl;
			return nullptr;
		}

		if( !dataHdr->size )
		{
			//std::cerr << "<error> array size is 0 -- header corrupted?" << std::endl;
			return nullptr;
		}

		bool isNormalized = dataHdr->flags & MF_NORMALIZED;

		size_t expectedSize = dataHdr->size * getElementSize( dt );
		size_t payloadBytes = hdr.dataBytes - sizeof( DataHdrArray );
		MsgFlags encoding = MF_ENCODING( dataHdr->flags );
		const void *values = getPayload( encoding, dataPayload, payloadBytes, expectedSize );
		if( !values )
		{
			//std::cerr << "<error> getting data failed" << std::endl;
			return nullptr;
		}

		float ratio = (float) payloadBytes / expectedSize;
		if( encoding == MF_NONE && payloadBytes != expectedSize )
		{
			//std::cerr << "<error> expected " << expectedSize << " bytes in uncompressed array (" << dataHdr->size << ") frame, but got " << hdr.dataBytes << std::endl;
			return nullptr;
		}

		//TODO: adjust normalization max values here (maybe have to add this to the data header, not sure if a hard-coded value is reasonable here)
		switch( MF_DATATYPE( dataHdr->flags ) )
		{
		case MF_DATATYPE_BYTE:
			frm = toFrame<unsigned char>( outType, dataHdr->size, 1, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 0xff );
			break;
		case MF_DATATYPE_INT16:
			frm = toFrame<int16_t>( outType, dataHdr->size, 1, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_INT32:
			frm = toFrame<int32_t>( outType, dataHdr->size, 1, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_FLOAT:
			frm = toFrame<float>( outType, dataHdr->size, 1, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
			break;
		default:
			//std::cerr << "<error> unknown data type" << std::endl;
			break;
		}

		sprintf( _desc, "%s, %s %.2f", dataTypeToString( dataHdr->flags ), encodingToString( dataHdr->flags ), ratio ); //TODO: add compression rate here

		return frm;
	}

	SampleFrame *ComMsgParser::createFrameFromMatrix( const ComMsgHdr &hdr, const void *data, DataType outType )
	{
		if( !data )
		{
			//std::cerr << "<error> data ptr must not be NULL" << std::endl;
			return nullptr;
		}

		SampleFrame *frm = nullptr;
		const DataHdrMatrix *dataHdr = reinterpret_cast<const DataHdrMatrix*>( data );
		const unsigned char *dataPayload = reinterpret_cast<const unsigned char*>( data ) + sizeof( DataHdrMatrix );

		MsgFlags dt = MF_DATATYPE( dataHdr->flags );
		if( dt != MF_DATATYPE_BYTE && dt != MF_DATATYPE_INT16 && dt != MF_DATATYPE_INT32 && dt != MF_DATATYPE_FLOAT )
		{
			//std::cerr << "<error> unknown data type -- header corrupted?" << std::endl;
			return nullptr;
		}

		size_t size = dataHdr->width * dataHdr->height;
		if( !size )
		{
			//std::cerr << "<error> matrix size is 0 -- header corrupted?" << std::endl;
			return nullptr;
		}

		bool isNormalized = dataHdr->flags & MF_NORMALIZED;

		size_t expectedSize = size * getElementSize( dt );
		size_t payloadBytes = hdr.dataBytes - sizeof( DataHdrMatrix );
		MsgFlags encoding = MF_ENCODING( dataHdr->flags );
		const void *values = getPayload( encoding, dataPayload, payloadBytes, expectedSize );
		if( !values )
		{
			//std::cerr << "<error> getting data failed" << std::endl;
			return nullptr;
		}

		float ratio = (float) payloadBytes / expectedSize;
		if( encoding == MF_NONE && payloadBytes != expectedSize )
		{
			//std::cerr << "<error> expected " << expectedSize << " bytes in uncompressed matrix (" << dataHdr->width << "x" << dataHdr->height << ") frame, but got " << hdr.dataBytes << std::endl;
			return nullptr;
		}

		//TODO: adjust normalization max values here (maybe have to add this to the data header, not sure if a hard-coded value is reasonable here)
		switch( MF_DATATYPE( dataHdr->flags ) )
		{
		case MF_DATATYPE_BYTE:
			frm = toFrame<unsigned char>( outType, dataHdr->width, dataHdr->height, 1, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
			break;
		case MF_DATATYPE_INT16:
			frm = toFrame<int16_t>( outType, dataHdr->width, dataHdr->height, 1, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_INT32:
			frm = toFrame<int32_t>( outType, dataHdr->width, dataHdr->height, 1, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_FLOAT:
			frm = toFrame<float>( outType, dataHdr->width, dataHdr->height, 1, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
			break;
		default:
			//std::cerr << "<error> unknown data type" << std::endl;
			break;
		}

		sprintf( _desc, "%s, %s %.2f", dataTypeToString( dataHdr->flags ), encodingToString( dataHdr->flags ), ratio ); //TODO: add compression rate here

		return frm;
	}

	SampleFrame* ComMsgParser::createFrameFromImage( const ComMsgHdr& hdr, const void* data, DataType outType )
	{
		if( !data )
		{
			//std::cerr << "<error> data ptr must not be NULL" << std::endl;
			return nullptr;
		}

		SampleFrame* frm = nullptr;
		const DataHdrImage* dataHdr = reinterpret_cast<const DataHdrImage*>( data );
		const unsigned char* dataPayload = reinterpret_cast<const unsigned char*>( data ) + sizeof( DataHdrImage );

		MsgFlags dt = MF_DATATYPE( dataHdr->flags );
		if( dt != MF_DATATYPE_BYTE && dt != MF_DATATYPE_INT16 && dt != MF_DATATYPE_INT32 && dt != MF_DATATYPE_FLOAT )
		{
			//std::cerr << "<error> unknown data type -- header corrupted?" << std::endl;
			return nullptr;
		}

		size_t size = dataHdr->width * dataHdr->height * dataHdr->depth;
		if( !size )
		{
			//std::cerr << "<error> image size is 0 -- header corrupted?" << std::endl;
			return nullptr;
		}

		bool isNormalized = dataHdr->flags & MF_NORMALIZED;

		size_t expectedSize = size * getElementSize( dt );
		size_t payloadBytes = hdr.dataBytes - sizeof( DataHdrImage );
		MsgFlags encoding = MF_ENCODING( dataHdr->flags );
		const void* values = getPayload( encoding, dataPayload, payloadBytes, expectedSize );
		if( !values )
		{
			//std::cerr << "<error> getting data failed" << std::endl;
			return nullptr;
		}

		float ratio = (float) payloadBytes / expectedSize;
		if( encoding == MF_NONE && payloadBytes != expectedSize )
		{
			//std::cerr << "<error> expected " << expectedSize << " bytes in uncompressed image (" << dataHdr->width << "x" << dataHdr->height << "x" << dataHdr->depth << ") frame, but got " << hdr.dataBytes << std::endl;
			return nullptr;
		}

		//TODO: adjust normalization max values here (maybe have to add this to the data header, not sure if a hard-coded value is reasonable here)
		switch( MF_DATATYPE( dataHdr->flags ) )
		{
		case MF_DATATYPE_BYTE:
			frm = toFrame<unsigned char>( outType, dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const unsigned char*>( values ), hdr.timeStamp, !isNormalized, 255 );
			break;
		case MF_DATATYPE_INT16:
			frm = toFrame<int16_t>( outType, dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const int16_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_INT32:
			frm = toFrame<int32_t>( outType, dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const int32_t*>( values ), hdr.timeStamp, !isNormalized, ( 0x01 << 10 ) - 1 );
			break;
		case MF_DATATYPE_FLOAT:
			frm = toFrame<float>( outType, dataHdr->width, dataHdr->height, dataHdr->depth, reinterpret_cast<const float*>( values ), hdr.timeStamp, !isNormalized, 1.0f );
			break;
		default:
			//std::cerr << "<error> unknown data type" << std::endl;
			break;
		}

		sprintf( _desc, "%s, %s %.2f", dataTypeToString( dataHdr->flags ), encodingToString( dataHdr->flags ), ratio ); //TODO: add compression rate here

		return frm;
	}
}