/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

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

    SampleFrame::SampleFrame( DataType type, size_t width, size_t height, size_t depth ) :
        _layout( sqid::getLayout( width, height, depth ) ),
        _type( type ),
        _width( width ),
        _height( height ),
        _depth( depth ),
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