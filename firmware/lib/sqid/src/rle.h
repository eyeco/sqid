#ifndef _SQID_RLE
#define _SQID_RLE

#include "sqid.h"

#include "stddef.h"

namespace Sqid
{
#ifdef _SQID_RLE_ENC_SUPPORT
    size_t encodeRLE( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax );
    size_t decompressRLE( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax );
#endif
}

#endif