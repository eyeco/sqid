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

#include <common.h>

namespace sqid
{
	class SQID_API BlobFrame
	{
	private:
		uint32_t _timeStamp;

		size_t _size;

		size_t _bufferSize;
		unsigned char *_buffer;

		void reserve( size_t size )
		{
			if( !size )
				return;

			if( size > _bufferSize )
			{
				safeDeleteArray( _buffer );

				_buffer = new unsigned char[size];
				_bufferSize = size;
			}
		}

	public:
		BlobFrame() :
			_timeStamp( 0 ),
			_bufferSize( 0 ),
			_buffer( nullptr )
		{}

		BlobFrame( size_t size, uint32_t timestamp ) :
			_timeStamp( timestamp ),
			_size( size ),
			_bufferSize( 0 ),
			_buffer( nullptr )
		{
			reserve( size );
		}

		BlobFrame( const unsigned char *data, size_t size, uint32_t timestamp ) :
			_timeStamp( timestamp ),
			_size( 0 ),
			_bufferSize( 0 ),
			_buffer( nullptr )
		{
			set( data, size );
		}

		BlobFrame( const BlobFrame &b ) :
			_timeStamp( b._timeStamp ),
			_size( 0 ),
			_bufferSize( 0 ),
			_buffer( nullptr )
		{
			set( b._buffer, b._size );
		}

		~BlobFrame()
		{
			safeDeleteArray( _buffer );
		}

		uint32_t timeStamp() const { return _timeStamp; }

		size_t size() const { return _size; }

		unsigned char *data() { return ( _bufferSize ? _buffer : nullptr ); }
		const unsigned char *data() const { return ( _bufferSize ? _buffer : nullptr ); }

		void set( const unsigned char *data, size_t size )
		{
			reserve( size );

			memcpy( _buffer, data, size );

			_size = size;
		}
	};

#ifdef __COMPRESSION_SUPPORT
	class SQID_API CompressedSampleFrame
	{
	private:
		size_t _width;
		size_t _height;
		size_t _depth;

		uint32_t _timeStamp;

		CompressionAlgorithm _algorithm;

		size_t _bytes;

		size_t _bufferBytes;
		unsigned char *_buffer;

		void reserve( size_t bytes )
		{
			if( !bytes )
				return;

			if( bytes > _bufferBytes )
			{
				safeDeleteArray( _buffer );

				_buffer = new unsigned char[bytes];
				_bufferBytes = bytes;
			}
		}

	public:
		CompressedSampleFrame( const unsigned char *data, size_t bytes, CompressionAlgorithm algorithm, size_t width, size_t height, size_t depth, uint32_t timeStamp ) :
			_width( 0 ),
			_height( 0 ),
			_depth( 0 ),
			_timeStamp( 0 ),
			_algorithm( CA_NULL ),
			_bytes( 0 ),
			_bufferBytes( 0 ),
			_buffer( nullptr )
		{
			set( data, bytes, algorithm, width, height, depth, timeStamp );
		}

		CompressedSampleFrame( const CompressedSampleFrame &b ) :
			_width( 0 ),
			_height( 0 ),
			_depth( 0 ),
			_timeStamp( 0 ),
			_algorithm( CA_NULL ),
			_bytes( 0 ),
			_bufferBytes( 0 ),
			_buffer( nullptr )
		{
			set( b._buffer, b._bytes, b._algorithm, b._width, b._height, b._depth, b._timeStamp );
		}

		~CompressedSampleFrame()
		{
			safeDeleteArray( _buffer );
		}

		CompressionAlgorithm algorithm() const { return _algorithm; }
		uint32_t timeStamp() const { return _timeStamp; }

		size_t bytes() const { return _bytes; }

		size_t width() const { return _width; }
		size_t height() const { return _height; }
		size_t depth() const { return _depth; }

		size_t frameSize() const { return _width * _height * _depth; }

		unsigned char *data() { return ( _bufferBytes ? _buffer : nullptr ); }
		const unsigned char *data() const { return ( _bufferBytes ? _buffer : nullptr ); }

		void set( const unsigned char *data, size_t bytes, CompressionAlgorithm algorithm, size_t width, size_t height, size_t depth, uint32_t timeStamp )
		{
			reserve( bytes );

			memcpy( _buffer, data, bytes );

			_algorithm = algorithm;
			_bytes = bytes;

			_width = width;
			_height = height;
			_depth = depth;

			_timeStamp = timeStamp;
		}
	};
#endif

	template<> std::string toString<BlobFrame>( const BlobFrame &bf );

#ifdef __COMPRESSION_SUPPORT
	template<> std::string toString<CompressedSampleFrame>( const CompressedSampleFrame &cbf );
#endif
}


SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::BlobFrame &f );

#ifdef __COMPRESSION_SUPPORT
SQID_API std::ostream& SQID_API_CALL operator << ( std::ostream &ostr, const sqid::CompressedSampleFrame &f );
#endif
