/*
    Abstraction class of the communication protocol.
    Takes care of the initial setup and the sending of current sensor data.    
    Authors: Roland Aigner, Andreas Pointner
    Version: 1.0
 */

#ifndef _SQID_COM_MSG
#define _SQID_COM_MSG

#include "common.h"

#include <stddef.h>
#include <inttypes.h>

namespace sqid
{
    enum ProtocolVersion : unsigned char
    {
        PV_2 = 2
    };

    #define PV_LATEST PV_2

    enum MsgType : unsigned char
    {
        MT_VALUE,
        MT_ARRAY,
        MT_MATRIX,
        MT_IMAGE,

        // MT_PROPERTY,

        MT_COUNT
    };

    enum MsgFlags : unsigned short
    {
        MF_NONE = 0,

        MF_NORMALIZED = 0x01 << 0,
        MF_RESERVED_0 = 0x01 << 1,
        MF_RESERVED_1 = 0x01 << 2,

        MF_DATATYPE_MASK = 0x03 << 8, // bits 8 and 9 specify data type

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

#define MF_DATATYPE( a ) ( (MsgFlags)( a & MF_DATATYPE_MASK ) )
#define MF_ENCODING( a ) ( (MsgFlags)( a & MF_ENC_MASK ) )

    #pragma pack(push)
    #pragma pack(1)

    struct ComMsgHdr
    {
        unsigned char ver; // protocol version

        MsgType type; // message type (see enum)

        unsigned char deviceID;
        unsigned char sensorID;

        uint32_t timeStamp; // milliseconds since startup

        uint16_t dataBytes; // stride of following data block, in bytes
    };

    struct ComMsgHdrEx
    {
        ComMsgHdr hdr;

        unsigned char chk; // header checksum
    };

    struct ComMsg
    {
        ComMsgHdrEx hdr;

        uint8_t *data;
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

    static const unsigned int SYNC_BYTES = 2;
    static const unsigned char syncBytes[SYNC_BYTES] = { 0xf7, 0x7f };

    inline uint8_t makeChkSum( const ComMsgHdr *hdr )
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

    size_t getElementSize( MsgFlags flags );

    const char *dataTypeToString( MsgFlags flags );
    const char *encodingToString( MsgFlags flags );
}
#endif
