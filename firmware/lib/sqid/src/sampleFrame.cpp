#include "sampleFrame.h"

#include <string.h>

namespace sqid
{
    size_t getDataTypeSize( SampleFrame::DataType t )
    {
        switch( t )
        {
        case SampleFrame::DT_BYTE:
            return sizeof( uint8_t );
        case SampleFrame::DT_USHORT:
            return sizeof( uint16_t );
        case SampleFrame::DT_ULONG:
            return sizeof( uint32_t );
        case SampleFrame::DT_FLOAT:
            return sizeof( float );
        default:
            break;
        }
        
        return 0;
    }

    SampleFrame::SampleFrame( unsigned char deviceID, unsigned char sensorID, Layout layout, DataType type, size_t width, size_t height, size_t depth ) :
        _deviceID( deviceID ),
        _sensorID( sensorID ),
        _layout( layout ),
        _type( type ),
        _width( width ),
        _height( height ),
        _depth( depth ),
        _elements( width * height * depth ),
        _elemSize( getDataTypeSize( type ) ),
        _size( _elements * _elemSize ),
        _data( _size ? new uint8_t[_size] : nullptr )
    {
    }

    SampleFrame::~SampleFrame()
    {
        safeDeleteArray( _data );
    }

    bool SampleFrame::setData( const uint8_t *data )
    {
        if( !data || !_data )
            return false;

        memcpy( _data, data, _size );

        return true;
    }
}