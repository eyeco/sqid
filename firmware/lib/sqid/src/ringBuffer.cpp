/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#include "ringBuffer.h"

#include <iostream>

namespace sqid
{
    RingBuffer::RingBuffer( size_t capacity ) :
        _size( 0 ),
        _capacity( 0 ),
        _buffer( nullptr ),
        _end( nullptr ),
        _head( nullptr ),
        _tail( nullptr )
    {
        reserve( capacity, false );
    }

    RingBuffer::~RingBuffer()
    {
        safeDeleteArray( _buffer );

        _end = nullptr;

        _head = nullptr;
        _tail = nullptr;
    }

    void RingBuffer::reserve( size_t capacity, bool keepData )
    {
        if( capacity <= 0 || capacity < _size )
        {
            std::cerr << "<error> invalid argument for capacity: " << capacity << std::endl;
            return;
        }

        if( capacity == _size )
            return;

        uint8_t *oldBuffer = _buffer;
        uint8_t *oldEnd = _end;
        uint8_t *oldHead = _head;
        uint8_t *oldTail = _tail;

        _size = 0;
        _capacity = capacity;

        _buffer = new uint8_t[_capacity];
        _end = _buffer + _capacity;
        _head = _buffer;
        _tail = _buffer;

        if( oldBuffer )
        {
            if( keepData )
            {
                if( oldTail > oldHead )
                    add( oldHead, oldTail );
                else
                {
                    add( oldHead, oldEnd );
                    add( oldBuffer, oldTail );
                }
            }

            safeDeleteArray( oldBuffer );
        }
    }

    void RingBuffer::add( const uint8_t t ) 
    {
        if( isFull() )
        {
            size_t newSize = nextPo2( _capacity * 2 );
            std::cerr << "<warning> ringbuffer is full, resizing to " << newSize << std::endl;
            reserve( newSize, true );
        }

        *( _tail++ ) = t;
        if( _tail == _end )
            _tail = _buffer;

        _size++;
    }

    void RingBuffer::add( const uint8_t *start, const uint8_t *end )
    {
        if( start == end )
            return;

        add( start, end - start );
    }

    void RingBuffer::add( const uint8_t *start, size_t count )
    {
        if( !count )
            return;

        if( _size + count > _capacity )
        {
            size_t newSize = nextPo2( _size + count );
            std::cerr << "<warning> ringbuffer is full, resizing to " << newSize << std::endl;
            reserve( newSize, true );
        }

        if( _tail + count > _end )
        {
            size_t count1 = _end - _tail;
            size_t count2 = count - count1;;
            memcpy( _tail, start, count1 );
            memcpy( _buffer, start + count1, count2 );
            _tail = _buffer + count2;
        }
        else
        {
            memcpy( _tail, start, count );
            _tail += count;
        }

        if( _tail == _end )
            _tail = _buffer;

        _size += count;
    }

    bool RingBuffer::tryAdd( const uint8_t &t )
    {
        if( isFull() )
            return false;

        add( t );

        return true;
    }

    bool RingBuffer::tryAdd( const uint8_t *start, const uint8_t *end )
    {
        if( start == end )
            return true;

        if( start > end )
            return false;

        return tryAdd( start, ( end - start ) );
    }

    bool RingBuffer::tryAdd( const uint8_t *start, size_t count )
    {
        if( !count )
            return true;

        if( _size + count > _capacity )
            return false;

        add( start, count );

        return true;
    }

    uint8_t RingBuffer::get()
    {
        const uint8_t *ret = ( _head++ );
        if( _head == _end )
            _head = _buffer;

        _size--;

        return *ret;
    }

    void RingBuffer::get( uint8_t *start, size_t count )
    {
        //assert( count <= _size && "count <= size" );

        if( _head + count > _end )
        {
            size_t count1 = _end - _head;
            size_t count2 = count - count1;
            memcpy( start, _head, count1 );
            memcpy( start + count1, _buffer, count2 );
            _head = _buffer + count2;
        }
        else
        {
            memcpy( start, _head, count );
            _head += count;
        }

        if( _head == _end )
            _head = _buffer;

        _size -= count;
    }

    uint8_t RingBuffer::peek()
    {
        //assert( _size > 0 && "size > 0" );

        return *_head;
    }

    void RingBuffer::peek( uint8_t *start, size_t count )
    {
        //assert( count <= _size && "count <= size" );

        //TODO: optimize, block copy
        const uint8_t *ptrIn = _head;

        for( int i = 0; i < count; i++ )
        {
            *( start++ ) = *( ptrIn++ );
            if( ptrIn == _end )
                ptrIn = _buffer;
        }
    }
}