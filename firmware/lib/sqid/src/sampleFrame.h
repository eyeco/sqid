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

        template<typename T>
        bool set( T t, size_t pos )
        {
            if( !_data || pos >= _elements )
                return false;

            uint8_t *ptr = _data + ( pos * _elemSize );
            switch( _type )
            {
            case DT_BYTE:
                *(reinterpret_cast<uint8_t*>( ptr )) = t;
                break;
            case DT_USHORT:
                *(reinterpret_cast<uint16_t*>( ptr )) = t;
                break;
            case DT_ULONG:
                *(reinterpret_cast<uint32_t*>( ptr )) = t;
                break;
            case DT_FLOAT:
                *(reinterpret_cast<float*>( ptr )) = t;
                break;
            default:
                return false;
            }

            return true;
        }

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

    template<typename T>
    inline SampleFrame *toFrame( DataType type, unsigned int width, unsigned int height, unsigned int depth, const T *values, bool normalize, T maxValue, bool clamp = false )
    {
        if( !values )
            return nullptr;

        SampleFrame *frame = new SampleFrame( type, width, height, depth );

        int size = width * height * depth;

        if( normalize )
        {
            float s = 1.0f / maxValue;
            if( clamp )
            {
                for( int i = 0; i < size; i++ )
                    frame->set<T>( sqid::clamp<float>( values[i] * s, 0.0f, 1.0f ), i );
            }
            else
            {
                for( int i = 0; i < size; i++ )
                    frame->set<T>( values[i] * s, i );
            }
        }
        else
            for( int i = 0; i < size; i++ )
                frame->set<T>( values[i], i );

        return frame;
    }
}

#endif