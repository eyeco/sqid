/*
    parser class for converting serial protocol into sampleframe data structure
    Authors: Roland Aigner
    Version: 1.0
 */

#ifndef _SQID_COM_MSG_PARSER
#define _SQID_COM_MSG_PARSER

#include "comMsg.h"

namespace sqid
{
    class SampleFrame;

 	class ComMsgParser
	{
	private:
		char _desc[128];

        const unsigned char *getPayload( MsgFlags encoding, const unsigned char *data, size_t size, size_t expectedSize );

	public:
		ComMsgParser();
		~ComMsgParser();

		const char *getDesc() const { return _desc; }

		SampleFrame *createFrameFromSingleValue( const ComMsgHdr &hdr, const void *data, DataType outType );
		SampleFrame *createFrameFromArray( const ComMsgHdr &hdr, const void *data, DataType outType );
		SampleFrame *createFrameFromMatrix( const ComMsgHdr &hdr, const void *data, DataType outType );
		SampleFrame *createFrameFromImage( const ComMsgHdr &hdr, const void *data, DataType outType );
	};
}

#endif