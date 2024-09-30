/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <config.h>
#include <common.h>

namespace sqid
{
#ifdef __COMPRESSION_SUPPORT
	namespace Internal
	{
		class DecompressorImpl;
		class ImageDecompressorImpl;
	}

	class Decompressor
	{
	protected:
		Internal::DecompressorImpl *impl;

		explicit Decompressor( Internal::DecompressorImpl *impl );
	public:
		virtual ~Decompressor();

		size_t decompress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytes, bool silent = false );

		virtual CompressionAlgorithm getAlgorithm() const { return CA_NULL; }
	};

	class DecompressorDummy : public Decompressor
	{
	public:
		DecompressorDummy();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_NULL; }
	};

	class DecompressorRLE : public Decompressor
	{
	public:
		DecompressorRLE();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_RLE; }
	};

#ifdef __COMPRESSION_SUPPORT_LZO
	class DecompressorLZO : public Decompressor
	{
	public:
		DecompressorLZO();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_LZO; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_QLZ
	class DecompressorQLZ : public Decompressor
	{
	public:
		DecompressorQLZ();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_QLZ; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_BZ2
	class DecompressorBZ2 : public Decompressor
	{
	public:
		DecompressorBZ2();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_BZ2; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_ZSTD
	class DecompressorZStd : public Decompressor
	{
	public:
		DecompressorZStd();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_ZSTD; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_ZLIB
	class DecompressorZLib : public Decompressor
	{
	public:
		DecompressorZLib();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_ZLIB; }
	};
#endif

#ifdef __COMPRESSION_SUPPORT_LZ4
	class DecompressorLZ4 : public Decompressor
	{
	public:
		DecompressorLZ4();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_LZ4; }
	};
#endif


	class ImageDecompressor
	{
	protected:
		Internal::ImageDecompressorImpl *_impl;

		explicit ImageDecompressor( Internal::ImageDecompressorImpl *impl );
	public:
		virtual ~ImageDecompressor();

		size_t decompress( const unsigned char *inData, size_t inBytes, size_t &width, size_t &height, size_t &depth, size_t &bpp, unsigned char *outData, size_t outBytes, bool silent = false );

		virtual CompressionAlgorithm getAlgorithm() const { return CA_NULL; }
	};

#ifdef __COMPRESSION_SUPPORT_JPEG
	class ImageDecompressorJpeg : public ImageDecompressor
	{
	public:
		ImageDecompressorJpeg();

		virtual CompressionAlgorithm getAlgorithm() const { return CA_JPEG; }
	};
#endif
#else
	//suppress warnings
	class Decompressor
	{};

	//suppress warnings
	class ImageDecompressor
	{};
#endif
}
