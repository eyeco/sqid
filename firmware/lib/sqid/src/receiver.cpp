#include "receiver.h"

#include "common.h"
#include "transport.h"

#include <iostream>

namespace Sqid
{
    ReceiverSerial::ReceiverSerial( size_t maxQueueSize ) :
        _buffer( 1024 ),
        _isSynced( false ),
        _currentState( PS_SYNCING ),
        _expectedBytes( 0 ),
        _maxQueueSize( maxQueueSize )
    {}

    ReceiverSerial::~ReceiverSerial()
    {}

    bool ReceiverSerial::update()
    {
        size_t size = TransportSerial::singleton().available();

        if(!size)
            return false;

        std::vector<uint8_t> buffer( size );
        uint8_t *data = &buffer[0];
        if( TransportSerial::singleton().readBytes( data, size ) != size )
            return false;

        _buffer.add( data, size );

        bool repeat = false;

        do
        {
            repeat = false;

            if( _currentState == PS_SYNCING )
            {
                while( _buffer.size() >= _expectedBytes )
                {
                    unsigned char bytes[SYNC_BYTES];
                    _buffer.peek( bytes, SYNC_BYTES );

                    if( !memcmp( syncBytes, bytes, SYNC_BYTES ) )
                    {
                        _buffer.get( bytes, SYNC_BYTES );

                        if( !_isSynced )
                        {
                            _isSynced = true;
                            std::cout << "synced serial port" << std::endl;
                        }

                        _currentState = PS_HEADER;
                        _expectedBytes = sizeof( ComMsgHdrEx );

                        break;
                    }
                    else
                        _buffer.get();	// move forward by one byte
                }
            }

            if( _currentState == PS_HEADER )
            {
                if( _buffer.size() >= _expectedBytes )
                {
                    _buffer.peek( reinterpret_cast<unsigned char*>( &_hdrEx ), sizeof( ComMsgHdrEx ) );

                    if( !checkHdr( &_hdrEx ) )
                    {
                        std::cerr << "<error> checksum of header corrupt -- maybe out of sync, trying to resync..." << std::endl;
                        //NOTE: ALSO CHECK YOUR STRUCT DATA PACKING ALIGNMENT AT RECEIVER AND SENDER SIDE IN THIS CASE!!

                        _isSynced = false;
                        _currentState = PS_SYNCING;
                        _expectedBytes = SYNC_BYTES;

                        repeat = true;
                    }
                    else
                    {
                        //discard previously peeked bytes
                        _buffer.get( reinterpret_cast<unsigned char*>( &_hdrEx ), sizeof( ComMsgHdrEx ) );

                        _currentState = PS_PAYLOAD;
                        _expectedBytes = _hdrEx.hdr.dataBytes;

                        if( _dataBuffer.size() < _expectedBytes )
                            _dataBuffer.resize( nextPo2( _expectedBytes ) );
                    }
                }
            }

            if( _currentState == PS_PAYLOAD )
            {
                if( _buffer.size() >= _expectedBytes )
                {
                    _buffer.peek( &_dataBuffer[0], _expectedBytes );

                    bool valid = true;
                    //TODO: check data integrity
                    if( !valid )
                    {
                        std::cerr << "<error> payload data seems to be corrupt -- maybe out of sync, trying to resync..." << std::endl;
                        _isSynced = false;
                        _currentState = PS_SYNCING;
                        _expectedBytes = SYNC_BYTES;
                    }
                    else
                    {
                        //discard previously peeked bytes
                        _buffer.get( &_dataBuffer[0], _expectedBytes );

                        uint8_t *msgData = new uint8_t[_expectedBytes];
                        memcpy( msgData, &_dataBuffer[0], _expectedBytes );

                        ComMsg msg;
                        msg.hdr = _hdrEx;
                        msg.data = msgData;

                        _msgQueue.push_back( msg );

                        while( _msgQueue.size() > _maxQueueSize )
                        {
                            safeDeleteArray( _msgQueue.front().data );
                            _msgQueue.pop_front();
                        }

                        _currentState = PS_SYNCING;
                        _expectedBytes = SYNC_BYTES;
                    }

                    repeat = true;
                }
            }
        }while( repeat );

        return true;
    }

    ComMsg ReceiverSerial::dequeue()
    {
        ComMsg ret = _msgQueue.front();
        _msgQueue.pop_front();
        return ret;
    }
}