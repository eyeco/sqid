#include "serialStreamBuf.h"

#include "transport.h"

#include "stdio.h"

namespace sqid
{
    int SerialStreamBuf::overflow(int c)
    {
        if (c == EOF)
        {
            return !EOF;
        }
        else
        {
            uint8_t cc = c;
            TransportSerial::singleton().write(&cc, 1);
            return c;
        }
    }
}