#include "ComMsg.h"


namespace sqid
{
    size_t getElementSize( MsgFlags flags )
    {
        switch( MF_DATATYPE( flags ) )
        {
          case MF_DATATYPE_BYTE:
              return sizeof( uint8_t );
          case MF_DATATYPE_INT16:
              return sizeof( uint16_t );
          case MF_DATATYPE_INT32:
              return sizeof( uint32_t );
          case MF_DATATYPE_FLOAT:
              return sizeof( float );
          default:
              break;
        }
        
        return 0;
    }

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
}