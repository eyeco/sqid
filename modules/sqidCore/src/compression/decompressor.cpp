/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "decompressor.h"

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
		class DecompressorImpl
		{
		private:

		public:
			DecompressorImpl() {}
			virtual ~DecompressorImpl() {}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent ) = 0;
		};

		class ImageDecompressorImpl
		{
		private:

		public:
			ImageDecompressorImpl() {}
			virtual ~ImageDecompressorImpl() {}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, size_t &width, size_t &height, size_t &depth, size_t &bpp, unsigned char *outData, size_t outBytesMax, bool silent ) = 0;
		};


		class DecompressorImplDummy : public DecompressorImpl
		{
		public:
			DecompressorImplDummy() {}
			virtual ~DecompressorImplDummy() {}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !outBytesMax )
					return 0;

				if( outBytesMax < inBytes )
					return 0;

				memcpy( outData, inData, inBytes );

				return inBytes;
			}
		};


		class DecompressorImplRLE : public DecompressorImpl
		{
		public:
			DecompressorImplRLE() :
				DecompressorImpl()
			{
				std::cout << "initialized decompressor with RLE" << std::endl;
			}

			virtual ~DecompressorImplRLE()
			{}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !inBytes || !outBytesMax )
					return 0;

				const unsigned char *inDataPtr = inData;
				const unsigned char *inDataPtrEnd = inData + inBytes;
				unsigned char *outDataPtr = outData;

				size_t outBytes = 0;
				unsigned char c = 0;
				unsigned char v = 0;
				while( inDataPtr < inDataPtrEnd )
				{
					c = *( inDataPtr++ );
					v = *( inDataPtr++ );

					if( outBytes + c > outBytesMax )
					{
						if( !silent )
							std::cerr << "<error> failed to decompress -- output buffer too small" << std::endl;
						return 0;
					}

					memset( outDataPtr, v, c );
					outDataPtr += c;
					outBytes += c;
				}

				size_t decompressedBytes = ( outDataPtr - outData );

				if( outBytesMax < decompressedBytes && !silent )
					std::cerr << "<error> exceeded output buffer length -- segfault!" << std::endl;

				return decompressedBytes;
			}
		};


#ifdef __COMPRESSION_SUPPORT_LZO
		class DecompressorImplLZO : public DecompressorImpl
		{
		private:

		public:
			DecompressorImplLZO() :
				DecompressorImpl()
			{
				std::cout << "initialized decompressor with LZO" << lzo_version_string() << std::endl;
			}

			virtual ~DecompressorImplLZO()
			{}
						
			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !inBytes || !outBytesMax )
					return 0;

				size_t decompressedBytes = 0;
				int err = lzo1x_decompress_safe( inData, inBytes, outData, &decompressedBytes, nullptr );
				if( err != LZO_E_OK )
				{
					if( !silent )
						std::cerr << "<error> internal LZO error -- decompression failed with error #" << err << std::endl;
					return 0;
				}

				if( outBytesMax < decompressedBytes && !silent )
					std::cerr << "<error> exceeded output buffer length -- segfault!" << std::endl;

				return decompressedBytes;
			}
		};
#endif


#ifdef __COMPRESSION_SUPPORT_QLZ
		class DecompressorImplQLZ : public DecompressorImpl
		{
		private:
			qlz_state_decompress *decompressState;

		public:
			DecompressorImplQLZ() :
				DecompressorImpl(),
				decompressState( (qlz_state_decompress *) malloc( sizeof( qlz_state_decompress ) ) ) //NOTE: *must* be allocated in heap, according to documentation!
			{
				std::cout << "initialized decompressor with QLZ " << QLZ_VERSION_MAJOR << "." << QLZ_VERSION_MINOR << "." << QLZ_VERSION_REVISION << std::endl;
			}

			virtual ~DecompressorImplQLZ()
			{
				if( decompressState )
				{
					free( decompressState );
					decompressState = nullptr;
				}
			}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !inBytes || !outBytesMax )
					return 0;

				size_t decompressedBytes = qlz_decompress( reinterpret_cast<const char*>( inData ), outData, decompressState );

				if( outBytesMax < decompressedBytes && !silent )
					std::cerr << "<error> exceeded output buffer length -- segfault!" << std::endl;

				return decompressedBytes;
			}
		};
#endif


#ifdef __COMPRESSION_SUPPORT_BZ2
		class DecompressorImplBZ2 : public DecompressorImpl
		{
		private:
			int small;
			int verbosity;

		public:
			DecompressorImplBZ2() :
				DecompressorImpl(), 
				small( 0 ),
				verbosity( 0 )
			{
				std::cout << "initialized decompressor with BZ2 " << BZ2_bzlibVersion() << std::endl;
			}

			virtual ~DecompressorImplBZ2()
			{}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !inBytes || !outBytesMax )
					return 0;

				size_t decompressedBytes = 0;

				unsigned int len = outBytesMax;
				int err = BZ2_bzBuffToBuffDecompress( reinterpret_cast<char*>( outData ), &len, const_cast<char*>( reinterpret_cast<const char*>( inData ) ), inBytes, small, verbosity );

				/*
				BZ_CONFIG_ERROR
				  if the library has been mis-compiled
				BZ_PARAM_ERROR
				  if dest is NULL or destLen is NULL
				  or small != 0 && small != 1
				  or verbosity < 0 or verbosity > 4
				BZ_MEM_ERROR
				  if insufficient memory is available
				BZ_OUTBUFF_FULL
				  if the size of the compressed data exceeds *destLen
				BZ_DATA_ERROR
				  if a data integrity error was detected in the compressed data
				BZ_DATA_ERROR_MAGIC
				  if the compressed data doesn't begin with the right magic bytes
				BZ_UNEXPECTED_EOF
				  if the compressed data ends unexpectedly
				BZ_OK
				  otherwise
				*/

				if( err != BZ_OK )
				{
					if( !silent )
						std::cerr << "<error> decompressing BZ2 data failed with error #" << err << std::endl;
					return 0;
				}

				decompressedBytes = len;

				if( outBytesMax < decompressedBytes && !silent )
					std::cerr << "<error> exceeded output buffer length -- segfault!" << std::endl;

				return decompressedBytes;
			}
		};
#endif


#ifdef __COMPRESSION_SUPPORT_ZSTD
		class DecompressorImplZStd : public DecompressorImpl
		{
		private:

		public:
			DecompressorImplZStd() :
				DecompressorImpl()
			{
				std::cout << "initialized decompressor with ZStd " << ZSTD_VERSION_STRING << std::endl;
			}

			virtual ~DecompressorImplZStd()
			{}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !inBytes || !outBytesMax )
					return 0;

				size_t decompressedBytes = ZSTD_decompress( outData, outBytesMax, inData, inBytes );
				if( ZSTD_isError( decompressedBytes ) )
				{
					if( !silent )
						std::cerr << "<error> decompression with ZStd failed with error #" << decompressedBytes << ": \"" << ZSTD_getErrorName( decompressedBytes ) << "\"" << std::endl;
					return 0;
				}

				if( outBytesMax < decompressedBytes && !silent )
					std::cerr << "<error> exceeded output buffer length -- segfault!" << std::endl;

				return decompressedBytes;
			}
		};
#endif


#ifdef __COMPRESSION_SUPPORT_ZLIB
		class DecompressorImplZLib : public DecompressorImpl
		{
		private:

		public:
			DecompressorImplZLib() :
				DecompressorImpl()
			{
				std::cout << "initialized decompressor with ZLib " << zlibVersion() << std::endl;
			}

			virtual ~DecompressorImplZLib()
			{}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !inBytes || !outBytesMax )
					return 0;

				size_t decompressedBytes = 0;
				
				z_stream zInfo = { 0 };
				zInfo.total_in = zInfo.avail_in = inBytes;
				zInfo.total_out = zInfo.avail_out = outBytesMax;
				zInfo.next_in = (BYTE*) inData;
				zInfo.next_out = outData;

				//NOTE: not sure if i have to re-initialize stream every time, maybe we can save some time here
				int nErr, nRet = -1;
				nErr = inflateInit( &zInfo );
				if( nErr == Z_OK )
				{
					nErr = inflate( &zInfo, Z_FINISH );
					if( nErr == Z_STREAM_END )
						nRet = zInfo.total_out;
				}
				inflateEnd( &zInfo );

				if( nErr < 0 )
				{
					if( !silent )
						std::cerr << "<error> ZLib error #" << nErr << std::endl;
					return 0;
				}

				if( nRet >= 0 )
					decompressedBytes = nRet;

				if( outBytesMax < decompressedBytes && !silent )
					std::cerr << "<error> exceeded output buffer length -- segfault!" << std::endl;

				return decompressedBytes;
			}
		};
#endif


#ifdef __COMPRESSION_SUPPORT_LZ4
		class DecompressorImplLZ4 : public DecompressorImpl
		{
		private:

		public:
			DecompressorImplLZ4() :
				DecompressorImpl()
			{
				std::cout << "initialized decompressor with LZ4 " << LZ4_versionString() << std::endl;
			}

			virtual ~DecompressorImplLZ4()
			{}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !inBytes || !outBytesMax )
					return 0;

				size_t decompressedBytes = 0;

				int ret = LZ4_decompress_safe( (const char*)inData, (char*)outData, inBytes, outBytesMax );

				if( ret < 0 )
				{
					if( !silent )
						std::cerr << "<error> decompression with LZ4 failed with error #" << ret << std::endl;
				}
				else
					decompressedBytes = ret;

				if( outBytesMax < decompressedBytes && !silent )
					std::cerr << "<error> exceeded output buffer length -- segfault!" << std::endl;

				return decompressedBytes;
			}
		};
#endif





#ifdef __COMPRESSION_SUPPORT_JPEG
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

		struct jpeg_error_mgr_ext
		{
			struct jpeg_error_mgr pub;    /* "public" fields */

			jmp_buf setjmp_buffer;        /* for return to caller */
		};

		typedef struct jpeg_error_mgr_ext* jpeg_error_mgr_ext_ptr;

		void my_error_exit( j_common_ptr cinfo )
		{
			/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
			jpeg_error_mgr_ext_ptr myerr = (jpeg_error_mgr_ext_ptr) cinfo->err;

			/* Always display the message. */
			/* We could postpone this until after returning, if we chose. */
			( *cinfo->err->output_message ) ( cinfo );

			/* Return control to the setjmp point */
			longjmp( myerr->setjmp_buffer, 1 );
		}


		class ImageDecompressorImplJpeg : public ImageDecompressorImpl
		{
		private:
			/* We use our private extension JPEG error handler.
			* Note that this struct must live as long as the main JPEG parameter
			* struct, to avoid dangling-pointer problems.
			*/
			struct jpeg_error_mgr_ext _jerr;

			/* This struct contains the JPEG decompression parameters and pointers to
			* working space (which is allocated as needed by the JPEG library).
			*/
			struct jpeg_decompress_struct _di;

			std::vector<unsigned char> _buffer;


			int decode( const unsigned char *inData, size_t inBytes, size_t &width, size_t &height, size_t &depth, size_t &bpp )
			{
				/* More stuff */
				JSAMPARRAY buffer;            /* Output row buffer */
				int rowStride;               /* physical row width in output buffer */

											 /* In this example we want to open the input file before doing anything else,
											 * so that the setjmp() error recovery below can assume the file is open.
											 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
											 * requires it in order to read binary files.
											 */


											 /* Step 2: specify data source (eg, a file) */

				jpeg_mem_src( &_di, const_cast<unsigned char*>( inData ), inBytes );

				/* Step 3: read file parameters with jpeg_read_header() */

				(void) jpeg_read_header( &_di, TRUE );
				/* We can ignore the return value from jpeg_read_header since
				*   (a) suspension is not possible with the stdio data source, and
				*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
				* See libjpeg.txt for more info.
				*/

				/* Step 4: set parameters for decompression */

				/* In this example, we don't need to change any of the defaults set by
				* jpeg_read_header(), so we do nothing here.
				*/

				/* Step 5: Start decompressor */

				(void) jpeg_start_decompress( &_di );
				/* We can ignore the return value since suspension is not possible
				* with the stdio data source.
				*/

				width = _di.output_width;
				height = _di.output_height;
				depth = _di.output_components;
				bpp = 8;

				int outSize = width * height * depth;

				if( _buffer.size() < outSize )
					_buffer.resize( outSize );

				/* We may need to do some setup of our own at this point before reading
				* the data.  After jpeg_start_decompress() we have the correct scaled
				* output image dimensions available, as well as the output colormap
				* if we asked for color quantization.
				* In this example, we need to make an output work buffer of the right size.
				*/
				/* JSAMPLEs per row in output buffer */
				rowStride = _di.output_width * _di.output_components;
				/* Make a one-row-high sample array that will go away when done with image */
				buffer = ( *_di.mem->alloc_sarray )
					( (j_common_ptr) &_di, JPOOL_IMAGE, rowStride, 1 );

				/* Step 6: while (scan lines remain to be read) */
				/*           jpeg_read_scanlines(...); */

				unsigned char *outDataPtr = &_buffer[0];

				/* Here we use the library's state variable cinfo.output_scanline as the
				* loop counter, so that we don't have to keep track ourselves.
				*/
				while( _di.output_scanline < _di.output_height )
				{
					/* jpeg_read_scanlines expects an array of pointers to scanlines.
					* Here the array is only one element long, but you could ask for
					* more than one scanline at a time if that's more convenient.
					*/
					(void) jpeg_read_scanlines( &_di, buffer, 1 );

					/* Assume put_scanline_someplace wants a pointer and sample count. */
					memcpy( outDataPtr, buffer[0], rowStride );
					outDataPtr += rowStride;
				}

				/* Step 7: Finish decompression */

				(void) jpeg_finish_decompress( &_di );
				/* We can ignore the return value since suspension is not possible
				* with the stdio data source.
				*/

				return outSize;
			}


		public:
			ImageDecompressorImplJpeg() :
				_jerr( { 0 } ),
				_di( { 0 } )
			{
				/* Step 1: allocate and initialize JPEG decompression object */

				/* We set up the normal JPEG error routines, then override error_exit. */
				_di.err = jpeg_std_error( &_jerr.pub );
				_jerr.pub.error_exit = my_error_exit;
				/* Establish the setjmp return context for my_error_exit to use. */
				if( setjmp( _jerr.setjmp_buffer ) )
				{
					/* If we get here, the JPEG code has signaled an error.
					* We need to clean up the JPEG object, close the input file, and return.
					*/
				}

				/* Now we can initialize the JPEG decompression object. */
				jpeg_create_decompress( &_di );

				std::cout << "initialized decompressor with JPEG v" << JPEG_LIB_VERSION << " (libjpegturbo v" << TOSTRING( LIBJPEG_TURBO_VERSION ) << ")" << std::endl;
			}

			virtual ~ImageDecompressorImplJpeg()
			{
				/* Step 8: Release JPEG decompression object */

				/* This is an important step since it will release a good deal of memory. */
				jpeg_destroy_decompress( &_di );

				/* At this point you may want to check to see whether any corrupt-data
				* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
				*/

				/* And we're done! */
			}

			virtual size_t decompress( const unsigned char *inData, size_t inBytes, size_t &width, size_t &height, size_t &depth, size_t &bpp, unsigned char *outData, size_t outBytesMax, bool silent )
			{
				if( !inData || !outData || !inBytes || !outBytesMax )
					return 0;

				size_t decompressedBytes = 0;

				int ret = decode( inData, inBytes, width, height, depth, bpp );
				
				if( ret <= 0 )
				{
					if( !silent )
						std::cerr << "<error> JPEG decompression failed" << std::endl;
					return 0;
				}

				decompressedBytes = ret;

				if( decompressedBytes > outBytesMax )
				{
					if( !silent )
						std::cerr << "<error> output buffer too small (JPEG)" << std::endl;
					return 0;
				}

				memcpy( outData, &_buffer[0], decompressedBytes );

				return decompressedBytes;

			}
		};
#endif
	}


	Decompressor::Decompressor( Internal::DecompressorImpl *impl ) :
		impl( impl )
	{}

	Decompressor::~Decompressor()
	{
		safeDelete( impl );
	}

	size_t Decompressor::decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytesMax, bool silent )
	{
		return impl->decompress( inData, inBytes, outData, outBytesMax, silent );
	}

	DecompressorDummy::DecompressorDummy() :
		Decompressor( new Internal::DecompressorImplDummy() )
	{}


	DecompressorRLE::DecompressorRLE() :
		Decompressor( new Internal::DecompressorImplRLE() )
	{}


#ifdef __COMPRESSION_SUPPORT_LZO
	DecompressorLZO::DecompressorLZO() :
		Decompressor( new Internal::DecompressorImplLZO() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_QLZ
	DecompressorQLZ::DecompressorQLZ() :
		Decompressor( new Internal::DecompressorImplQLZ() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_BZ2
	DecompressorBZ2::DecompressorBZ2() :
		Decompressor( new Internal::DecompressorImplBZ2() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_ZSTD
	DecompressorZStd::DecompressorZStd() :
		Decompressor( new Internal::DecompressorImplZStd() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_ZLIB
	DecompressorZLib::DecompressorZLib() :
		Decompressor( new Internal::DecompressorImplZLib() )
	{}
#endif

#ifdef __COMPRESSION_SUPPORT_LZ4
	DecompressorLZ4::DecompressorLZ4() :
		Decompressor( new Internal::DecompressorImplLZ4() )
	{}
#endif

	


	ImageDecompressor::ImageDecompressor( Internal::ImageDecompressorImpl *impl ) :
		_impl( impl )
	{}

	ImageDecompressor::~ImageDecompressor()
	{
		safeDelete( _impl );
	}

	size_t ImageDecompressor::decompress( const unsigned char *inData, size_t inBytes, size_t &width, size_t &height, size_t &depth, size_t &bpp, unsigned char *outData, size_t outBytes, bool silent )
	{
		return _impl->decompress( inData, inBytes, width, height, depth, bpp, outData, outBytes, silent );
	}


#ifdef __COMPRESSION_SUPPORT_JPEG
	ImageDecompressorJpeg::ImageDecompressorJpeg() :
		ImageDecompressor( new Internal::ImageDecompressorImplJpeg() )
	{}
#endif
#endif
}
