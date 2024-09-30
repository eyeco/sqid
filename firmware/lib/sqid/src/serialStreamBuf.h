#ifndef _SQID_SERIAL_STREAM_BUF
#define _SQID_SERIAL_STREAM_BUF

#include <iostream>

namespace sqid
{
    class SerialStreamBuf : public std::streambuf
    {
    private:
        virtual int overflow(int c);
    };

    inline void redirect( std::ostream &ostr, SerialStreamBuf &buf )
    {
        ostr.rdbuf(&buf);
    }
}

#endif