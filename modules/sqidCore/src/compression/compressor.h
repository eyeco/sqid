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

#include <config.h>
#include <common.h>

namespace sqid
{
#ifdef __COMPRESSION_SUPPORT
	namespace Internal
	{
		class CompressorImpl;
		class ImageCompressorImpl;
	}

	class Compressor
	{
	protected:
		Internal::CompressorImpl *_impl;

		explicit Compressor( Internal::CompressorImpl *impl );
	public:
		virtual ~Compressor();

		size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytes, bool silent = false );
		size_t toSafeSize( size_t inSize ) const;

		virtual CompressionAlgorithm getAlgorithm() const { return CA_NULL; }
	};

	class CompressorDummy : public Compressor
	{
	public:
		CompressorDummy();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_NULL; }
	};

	class CompressorRLE : public Compressor
	{
	public:
		CompressorRLE();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_RLE; }
	};

#ifdef __COMPRESSION_SUPPORT_LZO
	class CompressorLZO : public Compressor
	{
	public:
		CompressorLZO();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_LZO; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_QLZ
	class CompressorQLZ : public Compressor
	{
	public:
		CompressorQLZ();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_QLZ; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_BZ2
	class CompressorBZ2 : public Compressor
	{
	public:
		CompressorBZ2();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_BZ2; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_ZSTD
	class CompressorZStd : public Compressor
	{
	public:
		CompressorZStd();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_ZSTD; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_ZLIB
	class CompressorZLib : public Compressor
	{
	public:
		CompressorZLib();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_ZLIB; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_LZ4
	class CompressorLZ4 : public Compressor
	{
	public:
		CompressorLZ4();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_LZ4; }
	};
#endif


	

	class ImageCompressor
	{
	protected:
		Internal::ImageCompressorImpl *_impl;

		explicit ImageCompressor( Internal::ImageCompressorImpl *impl );
	public:
		virtual ~ImageCompressor();

		size_t compress( const unsigned char *inData, size_t width, size_t height, size_t depth, size_t bpp, ColorSpace cs, unsigned char *outData, size_t outBytes, bool silent = false );

		virtual CompressionAlgorithm getAlgorithm() const { return CA_NULL; }
	};

#ifdef __COMPRESSION_SUPPORT_JPEG
	class ImageCompressorJpeg : public ImageCompressor
	{
	public:
		explicit ImageCompressorJpeg( float quality = 0.8f );

		void setQuality( float quality );
		float getQuality() const;

		virtual CompressionAlgorithm getAlgorithm() const { return CA_JPEG; }
	};
#endif
#else
	//suppress warnings
	class Compressor
	{};

	//suppress warnings
	class ImageCompressor
	{};
#endif
}