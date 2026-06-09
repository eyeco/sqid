#ifndef _SQID_SENDER
#define _SQID_SENDER

#include "common.h"
#include "comMsg.h"

#ifdef SUPPORT_OSC
#include <IPAddress.h>
#endif

namespace sqid
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
    };

    class SenderSerial : public Sender
    {
    public:
        explicit SenderSerial();
        virtual ~SenderSerial();

        virtual bool init( const SampleFrame *frame, unsigned char deviceID, unsigned char sensorID, MsgFlags encoding = MF_ENC_UNCOMPRESSED );

        virtual bool send();

        virtual int available();
        virtual int readBytes( uint8_t *data, size_t bytes );

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
    };

#ifdef SUPPORT_BLUETOOTH_SERIAL
    class SenderRFCOMM : public SenderSerial
    {
    public:
        explicit SenderRFCOMM();
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
        */

    private:
        IPAddress _ip;
        uint16_t _port;
 
        const SampleFrame *_frame;
    };
#endif

}

#endif