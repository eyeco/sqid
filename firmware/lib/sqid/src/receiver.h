/*
    Parser for data incoming in potentially incomplete chunks, e.g. via
    Serial interface
    Authors: Roland Aigner, Andreas Pointner
    Version: 1.0
 */

#ifndef _SQID_COM_MSG_PARSER
#define _SQID_COM_MSG_PARSER

#include "sqid.h"

#include "comMsg.h"
#include "ringBuffer.h"

#include <list>
#include <vector>


namespace Sqid
{
    class ReceiverSerial
    {
    private:
        enum ProtocolState
        {
            PS_SYNCING,
            PS_HEADER,
            PS_PAYLOAD,

            PS_COUNT
        };
        
        RingBuffer _buffer;
        std::vector<unsigned char> _dataBuffer;

        bool _isSynced;
        ProtocolState _currentState;
        size_t _expectedBytes;

        ComMsgHdrEx _hdrEx;

        size_t _maxQueueSize;
        std::list<ComMsg> _msgQueue;

    public:
        explicit ReceiverSerial( size_t maxQueueSize );
        ~ReceiverSerial();

        bool update();

        //NOTE: not threadsafe!!
        size_t getMsgQueueSize() const { return _msgQueue.size(); }
        ComMsg dequeue();
    };
}

#endif