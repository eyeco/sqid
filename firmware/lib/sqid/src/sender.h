#ifndef _SQID_SENDER
#define _SQID_SENDER

#include "sqid.h"
#include "comMsg.h"

#ifdef SUPPORT_OSC
#include <IPAddress.h>
#endif

namespace Sqid
{
    class SampleFrame;
    class Compressor;

    class Sender
    {
    public:
        Sender() {}
        virtual ~Sender() {}

        virtual bool init( const SampleFrame *frame, MsgFlags encoding = MF_ENC_UNCOMPRESSED ) { return false; }

        virtual bool send() = 0;

        virtual int available() { return 0; }
        virtual int readBytes( uint8_t *data, size_t bytes ) { return 0; }

        //NOTE: FW stuff is sID/dID specific, due to protocol specification -- merge Sender and FWProps somehow
        virtual bool sendFWDesc( const char *desc ) { return false; }
        virtual bool sendFWStatus( const char *status ) { return false; }
        virtual bool sendFWAck( const char *propName ) { return false; }
    };

    class SenderSerial : public Sender
    {
    public:
        explicit SenderSerial( ProtocolVersion protocolVersion = PV_LATEST );
        virtual ~SenderSerial();

        virtual bool init( const SampleFrame *frame, MsgFlags encoding = MF_ENC_UNCOMPRESSED );

        virtual bool send();

        virtual int available();
        virtual int readBytes( uint8_t *data, size_t bytes );

        //NOTE: FW stuff is sID/dID specific, due to protocol specification -- merge Sender and FWProps somehow
        virtual bool sendFWDesc( const char *desc );
        virtual bool sendFWStatus( const char *status );
        virtual bool sendFWAck( const char *propName );

        //static void onSetup();
        //static void writeSerial( const uint8_t *buffer, size_t size );

    protected:
        virtual bool write( const uint8_t *buffer, size_t size );

    private:
        ComMsg _comMsg;
        MsgFlags _flags;

        bool _err;

        uint16_t _inputDataBytes;
        uint8_t _headerBytes;
        uint16_t _payloadBytesMax;

        const SampleFrame *_frame;
        Compressor *_compressor;

        bool sendFWPackage( MsgType type, const char *data, size_t len );
    };

#ifdef SUPPORT_BLUETOOTH_SERIAL
    class SenderRFCOMM : public SenderSerial
    {
    public:
        explicit SenderRFCOMM( ProtocolVersion protocolVersion );
        virtual ~SenderRFCOMM();

        virtual int available();
        virtual int readBytes( uint8_t *data, size_t bytes );

    protected:
        virtual bool write( const uint8_t *buffer, size_t size );
    };
#endif


#ifdef SUPPORT_OSC
    class SenderOSC : public Sender
    {
    public:
        SenderOSC( const char *address, uint16_t port );
        virtual ~SenderOSC();

        virtual bool init( const SampleFrame *frame, MsgFlags encoding = MF_ENC_UNCOMPRESSED );

        virtual bool send();

        /*
        //TODO: implement two-way OSC communication
        virtual int available() { return 0; }
        virtual int readBytes( uint8_t *data, size_t bytes ) { return 0; }

        //TODO: implement FW stuff for OSC
        //NOTE: FW stuff is sID/dID specific, due to protocol specification -- merge Sender and FWProps somehow
        virtual bool sendFWDesc( const char *desc ) { return false; }
        virtual bool sendFWStatus( const char *status ) { return false; }
        virtual bool sendFWAck( const char *propName ) { return false; }
        */

    private:
        IPAddress _ip;
        uint16_t _port;
 
        const SampleFrame *_frame;
    };
#endif

}

#endif