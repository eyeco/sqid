/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <algorithm>

namespace sqid
{
	template<typename T>
	class RingBuffer
	{
	private:
		size_t _size;
		size_t _capacity;

		T *_buffer;
		T *_end;

		T *_head;
		T *_tail;

		// example data layout
		// b ... data entry
		// x ... empty slots
		// 
		// b b b b b b b b b b b x x x x x x x b b b b b b
		// |                     |             |           |
		// buffer                tail          head        end

		void reserve( size_t capacity, bool keepData )
		{
			assert( capacity > 0 && "capacity > 0" );
			assert( capacity > _size && "capacity > current size" );

			T *oldBuffer = _buffer;
			T *oldEnd = _end;
			T *oldHead = _head;
			T *oldTail = _tail;

			_size = 0;
			_capacity = capacity;

			_buffer = new T[_capacity];
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

	public:
		explicit RingBuffer( size_t capacity = 1024 * sizeof( T ) ) :
			_size( 0 ),
			_capacity( 0 ),
			_buffer( nullptr ),
			_end( nullptr ),
			_head( nullptr ),
			_tail( nullptr )
		{
			reserve( capacity, false );
		}

		~RingBuffer()
		{
			safeDeleteArray( _buffer );

			_end = nullptr;

			_head = nullptr;
			_tail = nullptr;
		}

		void add( const T &t ) 
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

		void add( const T *start, const T *end )
		{
			if( start == end )
				return;

			add( start, end - start );
		}

		void add( const T *start, size_t count )
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

		bool tryAdd( const T &t )
		{
			if( isFull() )
				return false;

			add( t );

			return true;
		}

		bool tryAdd( const T *start, const T *end )
		{
			if( start == end )
				return true;

			if( start > end )
				return false;

			return tryAdd( start, ( end - start ) );
		}

		bool tryAdd( const T *start, size_t count )
		{
			if( !count )
				return true;

			if( _size + count > _capacity )
				return false;

			add( start, count );

			return true;
		}

		T get()
		{
			const T *ret = ( _head++ );
			if( _head == _end )
				_head = _buffer;

			_size--;

			return *ret;
		}

		void get( T *start, size_t count )
		{
			assert( count <= _size && "count <= size" );

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

		T peek()
		{
			assert( _size > 0 && "size > 0" );

			return *_head;
		}

		void peek( T *start, size_t count )
		{
			assert( count <= _size && "count <= size" );

			//TODO: optimize, block copy
			const T *ptrIn = _head;

			for( int i = 0; i < count; i++ )
			{
				*( start++ ) = *( ptrIn++ );
				if( ptrIn == _end )
					ptrIn = _buffer;
			}
		}

		bool isFull() const { return ( _size == _capacity ); }

		size_t size() const { return _size; }
		size_t capacity() const { return _capacity; }
	};
}