/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "compressor.h"

#ifdef __COMPRESSION_SUPPORT
#ifdef __COMPRESSION_SUPPORT_LZO
#include <minilzo.h>
#endif
#ifdef __COMPRESSION_SUPPORT_QLZ
#include <quicklz.h>
#endif
#ifdef __COMPRESSION_SUPPORT_BZ2
#include <bzlib.h>
#endif
#ifdef __COMPRESSION_SUPPORT_ZSTD
#include <zstd.h>
#endif
#ifdef __COMPRESSION_SUPPORT_ZLIB
#include <zlib.h>
#endif
#ifdef __COMPRESSION_SUPPORT_LZ4
#include <lz4.h>
#endif
#ifdef __COMPRESSION_SUPPORT_JPEG
#include <jpeglib.h>
#endif
#endif

namespace sqid
{
#ifdef __COMPRESSION_SUPPORT
	namespace Internal
	{
		class CompressorImpl
		{
		private:

		public:
			CompressorImpl() {}
			virtual ~CompressorImpl() {}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent ) = 0;
			virtual size_t toSafeSize( size_t inSize ) const = 0;
		};

		class ImageCompressorImpl
		{
		private:

		public:
			ImageCompressorImpl() {}
			virtual ~ImageCompressorImpl() {}

			virtual size_t compress( const unsigned char *inData, size_t width, size_t height, size_t depth, size_t bpp, ColorSpace cs, unsigned char *outData, size_t outBytesMax, bool silent ) = 0;
		};



		class CompressorImplDummy : public CompressorImpl
		{
		public:
			CompressorImplDummy() {}
			virtual ~CompressorImplDummy() {}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !outBytesMax )
					return 0;

				if( outBytesMax < inBytes )
					return 0;

				memcpy( outData, inData, inBytes );

				return inBytes;
			}

			virtual size_t toSafeSize( size_t inSize ) const { return inSize; }
		};

		
		class CompressorImplRLE : public CompressorImpl
		{
		private:
			size_t encode( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax )
			{
				const unsigned char *inPtr = inData;
				const unsigned char *inPtrEnd = inData + inBytes;

				unsigned int dataCntr = 0;

				unsigned char *outPtr = outData;
				unsigned char *outPtrEnd = outData + outBytesMax;

				int cntr = 0;
				int recent = -1;
				while( inPtr < inPtrEnd )
				{
					int current = *( inPtr++ );

					if( recent >= 0 && ( recent != current || cntr == 255 ) )
					{
						if( outPtr + 1 < outPtrEnd )
						{
							*( outPtr++ ) = cntr;
							*( outPtr++ ) = recent;
						}

						dataCntr += 2;

						recent = -1;
						cntr = 1;
					}
					else
						cntr++;

					recent = current;
				}

				if( recent >= 0 )
				{
					if( outPtr + 1 < outPtrEnd )
					{
						*( outPtr++ ) = cntr;
						*( outPtr++ ) = recent;
					}

					dataCntr += 2;
				}

				return dataCntr;
			}
			
		public:
			CompressorImplRLE() : 
				CompressorImpl()
			{
				std::cout << "initialized compressor with RLE" << std::endl;
			}

			virtual ~CompressorImplRLE()
			{}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

#if _DEBUG
				if( outBytesMax < toSafeSize( inBytes ) )
					std::cerr << "<warning> output buffer size may be too small in case of incompressable block" << std::endl;
#endif

				size_t compressedBytes = encode( inData, inBytes, outData, outBytesMax );

				if( compressedBytes >= inBytes && !silent )
					std::cerr << "<warning> block contains incompressible data (RLE)" << std::endl;
				if( compressedBytes >= outBytesMax && !silent )
					std::cerr << "<fatal> exceeded output buffer size (RLE)" << std::endl;

				return compressedBytes;
			}

			virtual size_t toSafeSize( size_t inSize ) const { return inSize * 2; }
		};

#ifdef __COMPRESSION_SUPPORT_LZO
#define HEAP_ALLOC(var,size) lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

		//TODO: this is obviously not threadsafe!
		static HEAP_ALLOC( wrkmem, LZO1X_1_MEM_COMPRESS );

		class CompressorImplLZO : public CompressorImpl
		{
		private:

		public:
			CompressorImplLZO() :
				CompressorImpl()
			{
				std::cout << "initialized compressor with LZO " << lzo_version_string() << std::endl;
			}

			//taken from MiniLZO sample testmini.c:
			// "Because the input block may be incompressible, we must provide a little more output space in case that compression is not possible."
			// a proposed buffer size is the following:
			// #define OUT_LEN     (IN_LEN + IN_LEN / 16 + 64 + 3)
			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				size_t compressedBytes = 0;

				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

#if _DEBUG
				if( outBytesMax < toSafeSize( inBytes ) && !silent )
					std::cerr << "<warning> output buffer size may be too small in case of incompressable block" << std::endl;
#endif

				int err = lzo1x_1_compress( inData, inBytes, outData, &compressedBytes, wrkmem );
				if( err != LZO_E_OK )
				{
					if( !silent )
						std::cerr << "<error> internal LZO error -- compression failed with error #" << err << std::endl;
					return 0;
				}

				if( compressedBytes >= inBytes && !silent )
					std::cerr << "<warning> block contains incompressible data (LZO)" << std::endl;
				if( compressedBytes >= outBytesMax && !silent )
					std::cerr << "<fatal> exceeded output buffer size (LZO)" << std::endl;

				return compressedBytes;
			}

			virtual size_t toSafeSize( size_t inSize ) const { return ( inSize + inSize / 16 + 64 + 3 ); }
		};
#endif

#ifdef __COMPRESSION_SUPPORT_QLZ
		class CompressorImplQLZ : public CompressorImpl
		{
		private:
			qlz_state_compress *compressState;

		public:
			CompressorImplQLZ() :
				CompressorImpl(),
				compressState( (qlz_state_compress *) malloc( sizeof( qlz_state_compress ) ) )	//NOTE: *must* be allocated in heap, according to documentation!
			{
				std::cout << "initialized compressor with QLZ " << QLZ_VERSION_MAJOR << "." << QLZ_VERSION_MINOR << "." << QLZ_VERSION_REVISION << std::endl;
			}

			virtual ~CompressorImplQLZ()
			{
				if( compressState )
				{
					free( compressState );
					compressState = nullptr;
				}
			}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				size_t compressedBytes = 0;

				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;
				
				//from QLZ documentation:
				// "Always allocate size + 400 bytes for the destination buffer when compressing."
#if _DEBUG
				if( outBytesMax < toSafeSize( inBytes ) && !silent )
					std::cerr << "<warning> output buffer size may be too small in case of incompressable block" << std::endl;
#endif

				compressedBytes = qlz_compress( inData, reinterpret_cast<char*>( outData ), inBytes, compressState );

				if( compressedBytes >= inBytes && !silent )
					std::cerr << "<warning> block contains incompressible data (QLZ)" << std::endl;
				if( compressedBytes >= outBytesMax && !silent )
					std::cerr << "<fatal> exceeded output buffer size (QLZ)" << std::endl;

				return compressedBytes;
			}

			virtual size_t toSafeSize( size_t inSize ) const { return ( inSize + 400 ); }
		};
#endif

#ifdef __COMPRESSION_SUPPORT_BZ2
		class CompressorImplBZ2 : public CompressorImpl
		{
		private:
			int blockSize100k;
			int verbosity;
			int workFactor;

		public:
			CompressorImplBZ2() :
				CompressorImpl(),
				blockSize100k( 9 ),
				verbosity( 0 ),
				workFactor( 30 )
			{
				std::cout << "initialized compressor with BZ2 " << BZ2_bzlibVersion() << std::endl;
			}

			virtual ~CompressorImplBZ2()
			{}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				size_t compressedBytes = 0;

				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

				if( blockSize100k < 1 || blockSize100k > 9 || verbosity < 0 || verbosity > 4 || workFactor < 0 || workFactor > 250 )
					return 0;

				unsigned int len = outBytesMax;
				int err = BZ2_bzBuffToBuffCompress( reinterpret_cast<char*>( outData ), &len, const_cast<char*>( reinterpret_cast<const char*>( inData ) ), inBytes, blockSize100k, verbosity, workFactor );

				if( err == BZ_OUTBUFF_FULL )
				{
					if( !silent )
						std::cerr << "<error> block contains incompressible data (BZ2)" << std::endl;
					return 0;
				}
				if( err != BZ_OK )
				{
					if( !silent )
						std::cerr << "<error> BZ2 compression error #" << err << std::endl;
					return 0;
				}

				compressedBytes = len;

				if( compressedBytes >= inBytes && !silent )
					std::cerr << "<warning> block contains incompressible data (BZ2)" << std::endl;
				if( compressedBytes >= outBytesMax && !silent )
					std::cerr << "<fatal> exceeded output buffer size (BZ2)" << std::endl;

				return compressedBytes;
			}

			virtual size_t toSafeSize( size_t inSize ) const { return inSize; }
		};
#endif

#ifdef __COMPRESSION_SUPPORT_ZSTD
		class CompressorImplZStd : public CompressorImpl
		{
		private:

		public:
			CompressorImplZStd() :
				CompressorImpl()
			{
				std::cout << "initialized compressor with ZStd " << ZSTD_VERSION_STRING << std::endl;
			}

			virtual ~CompressorImplZStd()
			{}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				size_t compressedBytes = 0;

				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

#if _DEBUG
				if( outBytesMax < toSafeSize( inBytes ) && !silent )
					std::cerr << "<warning> output buffer size may be too small in case of incompressable block" << std::endl;
#endif

				//TODO: from comments in simple_compression.c: 
				// "If you are doing many compressions, you may want to reuse the context. See the multiple_simple_compression.c example."
				compressedBytes = ZSTD_compress( outData, outBytesMax, inData, inBytes, 1 );
				if( ZSTD_isError( compressedBytes ) )
				{
					if( !silent )
						std::cerr << "<error> compression with ZStd failed with error #" << compressedBytes << ": \"" << ZSTD_getErrorName( compressedBytes ) << "\"" << std::endl;
					return 0;
				}

				if( compressedBytes >= inBytes && !silent )
					std::cerr << "<warning> block contains incompressible data (ZStd)" << std::endl;
				if( compressedBytes >= outBytesMax && !silent )
					std::cerr << "<fatal> exceeded output buffer size (ZStd)" << std::endl;

				return compressedBytes;
			}

			virtual size_t toSafeSize( size_t inSize ) const { return ZSTD_compressBound( inSize ); }
		};
#endif

#ifdef __COMPRESSION_SUPPORT_ZLIB
		class CompressorImplZLib : public CompressorImpl
		{
		private:

		public:
			CompressorImplZLib() :
				CompressorImpl()
			{
				std::cout << "initialized compressor with ZLib " << zlibVersion() << std::endl;
			}

			virtual ~CompressorImplZLib()
			{
			}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				size_t compressedBytes = 0;

				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

				z_stream zInfo = { 0 };
				zInfo.total_in = zInfo.avail_in = inBytes;
				zInfo.total_out = zInfo.avail_out = outBytesMax;
				zInfo.next_in = (BYTE*) inData;
				zInfo.next_out = outData;

				//NOTE: not sure if i have to re-initialize stream every time, maybe we can save some time here
				int nErr, nRet = -1;
				nErr = deflateInit( &zInfo, Z_DEFAULT_COMPRESSION );
				if( nErr == Z_OK )
				{
					nErr = deflate( &zInfo, Z_FINISH );
					if( nErr == Z_STREAM_END )
						nRet = zInfo.total_out;
				}
				deflateEnd( &zInfo );

				if( nErr < 0 )
				{
					if( !silent )
						std::cerr << "<error> ZLib error #" << nErr << std::endl;
					return 0;
				}

				if( nRet >= 0 )
					compressedBytes = nRet;

				if( compressedBytes >= inBytes && !silent )
					std::cerr << "<warning> block contains incompressible data (ZLib)" << std::endl;
				if( compressedBytes >= outBytesMax && !silent )
					std::cerr << "<fatal> exceeded output buffer size (ZLib)" << std::endl;

				return compressedBytes;
			}

			virtual size_t toSafeSize( size_t inSize ) const { return inSize; }
		};
#endif

#ifdef __COMPRESSION_SUPPORT_LZ4
		class CompressorImplLZ4 : public CompressorImpl
		{
		private:

		public:
			CompressorImplLZ4() :
				CompressorImpl()
			{
				std::cout << "initialized compressor with LZ4 " << LZ4_versionString() << std::endl;
			}

			virtual ~CompressorImplLZ4()
			{
			}

			virtual size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				size_t compressedBytes = 0;

				if( !inData || !inBytes || !outData || !outBytesMax )
					return 0;

				if( inBytes > LZ4_MAX_INPUT_SIZE )
				{
					if( !silent )
						std::cerr << "<error> max input size exceeded (" << inBytes << ", max is " << LZ4_MAX_INPUT_SIZE << std::endl;
					return 0;
				}

#if _DEBUG
				if( outBytesMax < toSafeSize( inBytes ) && !silent )
					std::cerr << "<warning> output buffer size may be too small in case of incompressable block" << std::endl;
#endif

				int ret = LZ4_compress_default( (const char*)inData, (char*)outData, inBytes, outBytesMax );

				if( ret )
					compressedBytes = ret;
				else if( !silent )
					std::cerr << "<error> compression with LZ4 failed" << std::endl;

				if( compressedBytes >= inBytes && !silent )
					std::cerr << "<warning> block contains incompressible data (ZLib)" << std::endl;
				if( compressedBytes >= outBytesMax && !silent )
					std::cerr << "<fatal> exceeded output buffer size (ZLib)" << std::endl;

				return compressedBytes;
			}

			virtual size_t toSafeSize( size_t inSize ) const { return LZ4_compressBound( inSize ); }
		};
#endif




		class ImageCompressorImplDummy : public ImageCompressorImpl
		{
		public:
			ImageCompressorImplDummy() {}
			virtual ~ImageCompressorImplDummy() {}

			virtual size_t compress( const unsigned char *inData, size_t width, size_t height, size_t depth, size_t bpp, ColorSpace cs, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !outBytesMax )
					return 0;

				size_t size = width * height * depth * ( bpp >> 3 );

				if( outBytesMax < size )
					return 0;

				memcpy( outData, inData, size );

				return size;
			}
		};

#ifdef __COMPRESSION_SUPPORT_JPEG
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
		
		class ImageCompressorImplJpeg : public ImageCompressorImpl
		{
		private:
			float _quality;

			/* This struct represents a JPEG error handler.  It is declared separately
			* because applications often want to supply a specialized error handler
			* (see the second half of this file for an example).  But here we just
			* take the easy way out and use the standard error handler, which will
			* print a message on stderr and call exit() if compression fails.
			* Note that this struct must live as long as the main JPEG parameter
			* struct, to avoid dangling-pointer problems.
			*/
			struct jpeg_error_mgr _jerr;

			/* This struct contains the JPEG compression parameters and pointers to
			* working space (which is allocated as needed by the JPEG library).
			* It is possible to have several such structures, representing multiple
			* compression/decompression processes, in existence at once.  We refer
			* to any one struct (and its associated working data) as a "JPEG object".
			*/
			struct jpeg_compress_struct _ci;

			unsigned char *_buffer;

			int encode( const unsigned char *imageData, unsigned int imageWidth, unsigned int imageHeight, unsigned int imageChannels, unsigned int bpp, ColorSpace colorSpace, float quality )
			{
				const JSAMPLE *imageBuffer = reinterpret_cast<const JSAMPLE*>( imageData );
				J_COLOR_SPACE cs = JCS_UNKNOWN;
				int q = (int) ( clamp( quality, 0.0f, 1.0f ) * 100 );

				switch( colorSpace )
				{
				case CS_UNKNOWN:
					cs = JCS_UNKNOWN;
					break;
				case CS_GRAYSCALE:
					cs = JCS_GRAYSCALE;
					break;
				case CS_RGB:
					cs = JCS_RGB;
					break;
				case CS_YCbCr:
					cs = JCS_YCbCr;
					break;
				case CS_CMYK:
					cs = JCS_CMYK;
					break;
				case CS_YCCK:
					cs = JCS_YCCK;
					break;
				case CS_RGBX:
					cs = JCS_EXT_RGBX;
					break;
				case CS_BGR:
					cs = JCS_EXT_BGR;
					break;
				case CS_BGRX:
					cs = JCS_EXT_BGRX;
					break;
				case CS_XBGR:
					cs = JCS_EXT_XBGR;
					break;
				case CS_XRGB:
					cs = JCS_EXT_XRGB;
					break;
				case CS_RGBA:
					cs = JCS_EXT_RGBA;
					break;
				case CS_BGRA:
					cs = JCS_EXT_BGRA;
					break;
				case CS_ABGR:
					cs = JCS_EXT_ABGR;
					break;
				case CS_ARGB:
					cs = JCS_EXT_ARGB;
					break;
				case CS_RGB565:
					cs = JCS_RGB565;
					break;
				}

				JSAMPROW rowPointer[1]; /* pointer to JSAMPLE row[s] */
				int rowStride;               /* physical row width in image buffer */

											 /* Step 2: specify data destination (eg, a file) */
											 /* Note: steps 2 and 3 can be done in either order. */

											 //if( this->outBuffer.size() < imageWidth * imageHeight * imageChannels )
											 //	this->outBuffer.resize( imageWidth * imageHeight * imageChannels );

											 //unsigned char *outPtr = &this->outBuffer[0];

											 //NOTE: jpeg_mem_dest seems to allocate memory and to leave me in charge for freeing it. couldn't find in 
											 // documentation, but I got a huge memleak here, so I suppose it's true...
				if( _buffer )
					free( _buffer );

				/* Here we use the library-supplied code to send compressed data to a
				* stdio stream.  You can also write your own code to do something else.
				* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
				* requires it in order to write binary files.
				*/
				unsigned long outSize = 0;
				jpeg_mem_dest( &_ci, &_buffer, &outSize );

				/* Step 3: set parameters for compression */

				/* First we supply a description of the input image.
				* Four fields of the cinfo struct must be filled in:
				*/
				_ci.image_width = imageWidth;      /* image width and height, in pixels */
				_ci.image_height = imageHeight;
				_ci.input_components = imageChannels;  /* # of color components per pixel */
				_ci.in_color_space = cs;       /* colorspace of input image */
															  /* Now use the library's routine to set default compression parameters.
															  * (You must set at least cinfo.in_color_space before calling this,
															  * since the defaults depend on the source color space.)
															  */
				jpeg_set_defaults( &_ci );
				/* Now you can set any non-default parameters you wish to.
				* Here we just illustrate the use of quality (quantization table) scaling:
				*/
				jpeg_set_quality( &_ci, q, TRUE /* limit to baseline-JPEG values */ );

				/* Step 4: Start compressor */

				/* TRUE ensures that we will write a complete interchange-JPEG file.
				* Pass TRUE unless you are very sure of what you're doing.
				*/
				jpeg_start_compress( &_ci, TRUE );

				/* Step 5: while (scan lines remain to be written) */
				/*           jpeg_write_scanlines(...); */

				/* Here we use the library's state variable cinfo.next_scanline as the
				* loop counter, so that we don't have to keep track ourselves.
				* To keep things simple, we pass one scanline per call; you can pass
				* more if you wish, though.
				*/
				rowStride = imageWidth * imageChannels; /* JSAMPLEs per row in image_buffer */

				while( _ci.next_scanline < _ci.image_height )
				{
					/* jpeg_write_scanlines expects an array of pointers to scanlines.
					* Here the array is only one element long, but you could pass
					* more than one scanline at a time if that's more convenient.
					*/
					rowPointer[0] = const_cast<JSAMPLE*>( &imageBuffer[_ci.next_scanline * rowStride] );
					(void) jpeg_write_scanlines( &_ci, rowPointer, 1 );
				}

				/* Step 6: Finish compression */

				jpeg_finish_compress( &_ci );

				return outSize;
			}

		public:
			explicit ImageCompressorImplJpeg( float quality ) : 
				_quality( quality ),
				_jerr( { 0 } ),
				_ci( { 0 } ),
				_buffer( nullptr )
			{
				/* Step 1: allocate and initialize JPEG compression object */

				/* We have to set up the error handler first, in case the initialization
				* step fails.  (Unlikely, but it could happen if you are out of memory.)
				* This routine fills in the contents of struct jerr, and returns jerr's
				* address which we place into the link field in compressInfo.
				*/
				_ci.err = jpeg_std_error( &_jerr );

				/* Now we can initialize the JPEG compression object. */
				jpeg_create_compress( &_ci );

				std::cout << "initialized compressor with JPEG v" << JPEG_LIB_VERSION << " (libjpegturbo v" << TOSTRING( LIBJPEG_TURBO_VERSION ) << ")" << std::endl;
			}

			virtual ~ImageCompressorImplJpeg()
			{
				//NOTE: jpeg_mem_dest seems to allocate memory and to leave me in charge for freeing it. couldn't find in 
				// documentation, but I got a huge memleak here, so I suppose it's true...
				if( _buffer )
					free( _buffer );

				/* Step 7: release JPEG compression object */

				/* This is an important step since it will release a good deal of memory. */
				jpeg_destroy_compress( &_ci );

				/* And we're done! */
			}

			virtual size_t compress( const unsigned char *inData, size_t width, size_t height, size_t depth, size_t bpp, ColorSpace cs, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				return compress( inData, width, height, depth, bpp, cs, outData, outBytesMax, _quality, silent );
			}

			virtual size_t compress( const unsigned char *inData, size_t width, size_t height, size_t depth, size_t bpp, ColorSpace cs, unsigned char *outData, size_t outBytesMax, float quality, bool silent )
			{
				if( !inData || !outData || !outBytesMax )
					return 0;

				size_t compressedBytes = 0;

				int ret = encode( inData, width, height, depth, bpp, cs, quality );

				if( ret <= 0 )
				{
					if( !silent )
						std::cerr << "<error> JPEG compression failed" << std::endl;
					return 0;
				}

				compressedBytes = ret;

				if( compressedBytes > outBytesMax )
				{
					if( !silent )
						std::cerr << "<error> output buffer too small (JPEG)" << std::endl;
					return 0;
				}

				memcpy( outData, _buffer, compressedBytes );

				return compressedBytes;
			}

			void setQuality( float quality ) { _quality = quality; }
			float getQuality() const { return _quality; }
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

	size_t Compressor::compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
	{
		return _impl->compress( inData, inBytes, outData, outBytesMax, silent );
	}

	size_t Compressor::toSafeSize( size_t inSize ) const
	{
		return _impl->toSafeSize( inSize );
	}


	CompressorDummy::CompressorDummy() :
		Compressor( new Internal::CompressorImplDummy() )
	{}


	CompressorRLE::CompressorRLE() :
		Compressor( new Internal::CompressorImplRLE() )
	{}


#ifdef __COMPRESSION_SUPPORT_LZO
	CompressorLZO::CompressorLZO() :
		Compressor( new Internal::CompressorImplLZO() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_QLZ
	CompressorQLZ::CompressorQLZ() :
		Compressor( new Internal::CompressorImplQLZ() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_BZ2
	CompressorBZ2::CompressorBZ2() :
		Compressor( new Internal::CompressorImplBZ2() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_ZSTD
	CompressorZStd::CompressorZStd() :
		Compressor( new Internal::CompressorImplZStd() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_ZLIB
	CompressorZLib::CompressorZLib() :
		Compressor( new Internal::CompressorImplZLib() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_LZ4
	CompressorLZ4::CompressorLZ4() :
		Compressor( new Internal::CompressorImplLZ4() )
	{}
#endif


	ImageCompressor::ImageCompressor( Internal::ImageCompressorImpl *impl ) :
		_impl( impl )
	{}

	ImageCompressor::~ImageCompressor()
	{
		safeDelete( _impl );
	}

	size_t ImageCompressor::compress( const unsigned char *inData, size_t width, size_t height, size_t depth, size_t bpp, ColorSpace cs, unsigned char *outData, size_t outBytes, bool silent )
	{
		return _impl->compress( inData, width, height, depth, bpp, cs, outData, outBytes, silent );
	}


#ifdef __COMPRESSION_SUPPORT_JPEG
	ImageCompressorJpeg::ImageCompressorJpeg( float quality ) :
		ImageCompressor( new Internal::ImageCompressorImplJpeg( quality ) )
	{}

	void ImageCompressorJpeg::setQuality( float quality ) { dynamic_cast<Internal::ImageCompressorImplJpeg*>( _impl )->setQuality( quality ); }
	float ImageCompressorJpeg::getQuality() const { return dynamic_cast<Internal::ImageCompressorImplJpeg*>( _impl )->getQuality(); }
#endif
#endif
}
