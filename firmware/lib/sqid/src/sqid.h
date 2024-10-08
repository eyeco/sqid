#ifndef _SQID
#define _SQID

#include <stddef.h>

namespace sqid
{
    enum Layout
    {
        L_0D,
        L_1D,
        L_2D,
        L_3D,

        L_COUNT
    };

    enum DataType
    {
        DT_BYTE,
        DT_USHORT,
        DT_ULONG,
        DT_FLOAT,

        DT_COUNT
    };

    inline Layout getLayout( size_t width, size_t height, size_t depth )
    {
        if(!(width * height * depth))
            return L_COUNT;

        if(depth == 1)
        {
            if(height == 1)
            {
                if(width == 1)
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
}

#endif