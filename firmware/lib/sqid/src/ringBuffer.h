#ifndef _SQID_RING_BUFFER
#define _SQID_RING_BUFFER

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include "common.h"

namespace Sqid
{
	class RingBuffer
	{
	private:
		size_t _size;
		size_t _capacity;

		uint8_t *_buffer;
		uint8_t *_end;

		uint8_t *_head;
		uint8_t *_tail;

		// example data layout
		// b ... data entry
		// x ... empty slots
		// 
		// b b b b b b b b b b b x x x x x x x b b b b b b
		// |                     |             |           |
		// buffer                tail          head        end

		void reserve( size_t capacity, bool keepData );

	public:
		explicit RingBuffer( size_t capacity = 1024 * sizeof( uint8_t ) );
		~RingBuffer();

		void add( const uint8_t t );
		void add( const uint8_t *start, const uint8_t *end );
		void add( const uint8_t *start, size_t count );

		bool tryAdd( const uint8_t &t );
		bool tryAdd( const uint8_t *start, const uint8_t *end );
		bool tryAdd( const uint8_t *start, size_t count );

		uint8_t get();
		void get( uint8_t *start, size_t count );

		uint8_t peek();
		void peek( uint8_t *start, size_t count );

		bool isFull() const { return ( _size == _capacity ); }

		size_t size() const { return _size; }
		size_t capacity() const { return _capacity; }
	};
}

#endif