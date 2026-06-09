#include "sampleFrame.h"

#include <string.h>

namespace sqid
{
    size_t getDataTypeSize( DataType t )
    {
        switch( t )
        {
        case DT_BYTE:
            return sizeof( uint8_t );
        case DT_USHORT:
            return sizeof( uint16_t );
        case DT_ULONG:
            return sizeof( uint32_t );
        case DT_FLOAT:
            return sizeof( float );
        default:
            break;
        }
        
        return 0;
    }

    SampleFrame::SampleFrame( DataType type, size_t width, size_t height, size_t depth, uint32_t ts ) :
        _layout( sqid::getLayout( width, height, depth ) ),
        _type( type ),
        _width( width ),
        _height( height ),
        _depth( depth ),
        _ts( ts ),
        _elements( width * height * depth ),
        _elemSize( getDataTypeSize( type ) ),
        _size( _elements * _elemSize ),
        _data( _size ? new uint8_t[_size] : nullptr )
    {}

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