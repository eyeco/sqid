#include "compressor.h"

#include "stdlib.h"
#include "string.h"

#ifdef _SQID_RLE_ENC_SUPPORT
#include "rle.h"
#endif

#ifdef _SQID_LZO_ENC_SUPPORT
#define __AVR32__
#include "minilzo.h"
#endif

#ifdef _SQID_ZSTD_ENC_SUPPORT
#include "zstd.h"
#endif

#ifdef _SQID_LZ4_ENC_SUPPORT
#include "lz4.h"
#endif

namespace sqid
{
    namespace Internal
    {
     	class CompressorImpl
		{
		private:

		public:
			CompressorImpl() {}
			virtual ~CompressorImpl() {}

			virtual size_t getMinOutBufferSize( size_t inBytes ) const = 0;
			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax ) = 0;
		};

  		class CompressorImplDummy : public CompressorImpl
		{
		public:
			CompressorImplDummy() {}
			virtual ~CompressorImplDummy() {}

			virtual size_t getMinOutBufferSize( size_t inBytes ) const 
			{
				return inBytes;
			}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
			{
				if( !inData || !outData || !outBytesMax )
					return 0;

				if( outBytesMax < inBytes )
					return 0;

				if( outBytesMax < getMinOutBufferSize( inBytes ) )
					return 0;

				memcpy( outData, inData, inBytes );

				return inBytes;
			}
		};

#ifdef _SQID_RLE_ENC_SUPPORT
        class CompressorImplRLE : public CompressorImpl
		{
		private:
		
		public:
			CompressorImplRLE() : 
				CompressorImpl()
			{}

			virtual ~CompressorImplRLE()
			{}

			virtual size_t getMinOutBufferSize( size_t inBytes ) const 
			{
				return inBytes * 2;
			}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
			{
				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

				if( outBytesMax < getMinOutBufferSize( inBytes ) )
					return 0;

				return encodeRLE( inData, inBytes, outData, outBytesMax );
			}
		};
#endif

#ifdef _SQID_LZO_ENC_SUPPORT

#define HEAP_ALLOC(var,size) lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

#error there seems to be an issue due to limited memory which I was unable to resolve so far...
		//TODO: this is obviously not threadsafe!
		static HEAP_ALLOC( wrkmem, LZO1X_1_MEM_COMPRESS );

        class CompressorImplLZO : public CompressorImpl
		{
		private:
		
		public:
			CompressorImplLZO() : 
				CompressorImpl()
			{}

			virtual ~CompressorImplLZO()
			{}

			virtual size_t getMinOutBufferSize( size_t inBytes ) const 
			{
				return ( inBytes + inBytes / 16 + 64 + 3 );
			}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
			{
				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

				if( outBytesMax < getMinOutBufferSize( inBytes ) )
					return 0;

				lzo_uint compressedBytes = 0;
				int err = lzo1x_1_compress( inData, inBytes, outData, &compressedBytes, wrkmem );
				if( err != LZO_E_OK )
					return 0;

				return compressedBytes;
			}
		};
#endif

#ifdef _SQID_ZSTD_ENC_SUPPORT
        class CompressorImplZStd : public CompressorImpl
		{
		private:

		public:
			CompressorImplZStd() :
				CompressorImpl()
			{}

			~CompressorImplZStd()
			{}

			virtual size_t getMinOutBufferSize( size_t inBytes ) const
			{
				return ZSTD_compressBound( inBytes );
			}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
			{
				size_t compressedBytes = 0;

				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

				if( outBytesMax < getMinOutBufferSize( inBytes ) )
					return 0;

				//TODO: from comments in simple_compression.c: 
				// "If you are doing many compressions, you may want to reuse the context. See the multiple_simple_compression.c example."
				compressedBytes = ZSTD_compress( outData, outBytesMax, inData, inBytes, 1 );
				if( ZSTD_isError( compressedBytes ) )
					return 0;

				return compressedBytes;
			}
		};
#endif

#ifdef _SQID_LZ4_ENC_SUPPORT
        class CompressorImplLZ4 : public CompressorImpl
		{
		private:

		public:
			CompressorImplLZ4() :
				CompressorImpl()
			{}

			~CompressorImplLZ4()
			{}

			virtual size_t getMinOutBufferSize( size_t inBytes ) const
			{
				return LZ4_compressBound( inBytes );
			}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
			{
				size_t compressedBytes = 0;

				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

				if( outBytesMax < getMinOutBufferSize( inBytes ) )
					return 0;

				if( inBytes > LZ4_MAX_INPUT_SIZE )
					return 0;

#error this here crashes.
				int ret = LZ4_compress_default( (const char*)inData, (char*)outData, inBytes, outBytesMax );

				if( ret )
					compressedBytes = ret;
				else
					return 0;

				return compressedBytes;
			}
		};
#endif
    }


	Compressor::Compressor( Internal::CompressorImpl *impl ) :
		_impl( impl )
	{}

	Compressor::~Compressor()
	{
		safeDelete( _impl );
	}

	size_t Compressor::getMinOutBufferSize( size_t inBytes ) const
	{
		return _impl->getMinOutBufferSize( inBytes );
	}

  	size_t Compressor::compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
	{
		return _impl->compress( inData, inBytes, outData, outBytesMax );
	}


  	CompressorDummy::CompressorDummy() :
		Compressor( new Internal::CompressorImplDummy() )
	{}

#ifdef _SQID_RLE_ENC_SUPPORT
   	CompressorRLE::CompressorRLE() :
		Compressor( new Internal::CompressorImplRLE() )
	{}
#endif

#ifdef _SQID_LZO_ENC_SUPPORT
    CompressorLZO::CompressorLZO() :
	 	Compressor( new Internal::CompressorImplLZO() )
	{}
#endif

#ifdef _SQID_ZSTD_ENC_SUPPORT
    CompressorZStd::CompressorZStd() :
	 	Compressor( new Internal::CompressorImplZStd() )
	{}
#endif

#ifdef _SQID_LZ4_ENC_SUPPORT
    CompressorLZ4::CompressorLZ4() :
	 	Compressor( new Internal::CompressorImplLZ4() )
	{}
#endif
}