#ifndef _SQID_SAMPLEFRAME
#define _SQID_SAMPLEFRAME

#include "sqid.h"

#include <stddef.h>
#include <inttypes.h>

namespace Sqid
{
    class SampleFrame
    {
    public:
        enum Layout
        {
            L_POINT,
            L_ARRAY,
            L_MATRIX,

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

        SampleFrame( unsigned char deviceID, unsigned char sensorID, Layout layout, DataType type, size_t width = 1, size_t height = 1, size_t depth = 1 );
        ~SampleFrame();

        unsigned char getDeviceID() const { return _deviceID; }
        unsigned char getSensorID() const { return _sensorID; }

        size_t getWidth() const { return _width; }
        size_t getHeight() const { return _height; }
        size_t getDepth() const { return _depth; }
        size_t getElements() const { return _elements; }
        size_t getSize() const { return _size; }

        DataType getDataType() const { return _type; }
        Layout getLayout() const { return _layout; }

        uint8_t *getData() const { return _data; }
        bool setData( const uint8_t *values );


    private:
        unsigned char _deviceID;
        unsigned char _sensorID;

        Layout _layout;
        DataType _type;

        size_t _width;
        size_t _height;
        size_t _depth;
        size_t _elements;

        size_t _elemSize;
        size_t _size;

        uint8_t *_data;
    };
}

#endif