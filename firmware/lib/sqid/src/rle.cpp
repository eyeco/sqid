#include "rle.h"

#include "stdlib.h"
#include "string.h"

namespace sqid
{
#ifdef _SQID_RLE_ENC_SUPPORT
    size_t encodeRLE( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
    {
        const unsigned char *inPtr = inData;
        const unsigned char *inPtrEnd = inData + inBytes;

        unsigned char *outPtr = outData;

        int cntr = 0;
        int recent = -1;
        while( inPtr < inPtrEnd )
        {
            int current = *( inPtr++ );

            if( recent >= 0 && ( recent != current || cntr == 255 ) )
            {
                *( outPtr++ ) = cntr;
                *( outPtr++ ) = recent;

                recent = -1;
                cntr = 1;
            }
            else
                cntr++;

            recent = current;
        }

        if( recent >= 0 )
        {
            *( outPtr++ ) = cntr;
            *( outPtr++ ) = recent;
        }

        return ( outPtr - outData );
    }

    size_t decompressRLE( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
    {
        if( !inData || !outData || !inBytes || !outBytesMax )
            return 0;

        const unsigned char *inDataPtr = inData;
        const unsigned char *inDataPtrEnd = inData + inBytes;
        unsigned char *outDataPtr = outData;

        size_t outBytes = 0;
        unsigned char c = 0;
        unsigned char v = 0;
        while( inDataPtr < inDataPtrEnd )
        {
            c = *( inDataPtr++ );
            v = *( inDataPtr++ );

            if( outBytes + c > outBytesMax )
            {
                return 0;
            }

            memset( outDataPtr, v, c );
            outDataPtr += c;
            outBytes += c;
        }

        return ( outDataPtr - outData );
    }
#endif
}
