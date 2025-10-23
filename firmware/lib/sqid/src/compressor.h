/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#ifndef _SQID_COMPRESSOR
#define _SQID_COMPRESSOR

#include "common.h"

#include "stddef.h"

namespace sqid
{
	namespace Internal
	{
		class CompressorImpl;
	}

    class Compressor
	{
	protected:
		Internal::CompressorImpl *_impl;

		explicit Compressor( Internal::CompressorImpl *impl );
	public:
		virtual ~Compressor();

		size_t getMinOutBufferSize( size_t inBytes ) const;
		size_t compress( const unsigned char *inData, size_t inBytes, unsigned char *outData, size_t outBytes );
	};

	class CompressorDummy : public Compressor
	{
	private:

	public:
		CompressorDummy();
	};

#ifdef _SQID_RLE_ENC_SUPPORT
	class CompressorRLE : public Compressor
	{
	private:

	public:
		CompressorRLE();
	};
#endif

#ifdef _SQID_LZO_ENC_SUPPORT
	class CompressorLZO : public Compressor
	{
	private:

	public:
		CompressorLZO();
	};
#endif

#ifdef _SQID_ZSTD_ENC_SUPPORT
	class CompressorZStd : public Compressor
	{
	private:

	public:
		CompressorZStd();
	};
#endif

#ifdef _SQID_LZ4_ENC_SUPPORT
	class CompressorLZ4 : public Compressor
	{
	private:

	public:
		CompressorLZ4();
	};
#endif
}

#endif