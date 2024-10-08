#ifndef _SQID_SAMPLEFRAME
#define _SQID_SAMPLEFRAME

#include "common.h"

#include <stddef.h>
#include <inttypes.h>

namespace sqid
{
    class SampleFrame
    {
    public:

        explicit SampleFrame( DataType type = DT_BYTE, size_t width = 1, size_t height = 1, size_t depth = 1 );
        ~SampleFrame();

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