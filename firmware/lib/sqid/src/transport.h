#ifndef _SQID_TRANSPORT
#define _SQID_TRANSPORT

#include "sqid.h"

#include "stdint.h"
#include "stdlib.h"

#ifdef SUPPORT_BLUETOOTH_SERIAL
#include "BluetoothSerial.h"
#endif

#ifdef SUPPORT_OSC
#include <WiFi.h>
#include <WiFiUdp.h>

#include "WiFi_SSIDnPWD.h"

#include "OSCBundle.h"
#include "OSCTiming.h"
#endif


namespace Sqid
{
    class TransportSerial
    {
    public:
        TransportSerial();
        ~TransportSerial();

        int available();

        size_t readBytes( uint8_t *buffer, size_t length );
        size_t write( const uint8_t *buffer, size_t size );

        static TransportSerial &singleton();
    };

#ifdef SUPPORT_BLUETOOTH_SERIAL
    class TransportRFCOMM
    {
    private:
        static BluetoothSerial SerialBT;

    public:
        TransportRFCOMM();
        ~TransportRFCOMM();

        int available();
        size_t readBytes( uint8_t *buffer, size_t length );
        size_t write( const uint8_t *buffer, size_t size );

        static TransportRFCOMM &singleton();
    };

    BluetoothSerial TransportRFCOMM::SerialBT;
#endif

#ifdef SUPPORT_OSC
    class TransportWifiUDP
    {
    private:
        static WiFiUDP udp;
        static bool connected;

        static const uint16_t OUT_PORT = 8888;

        static void connectToWiFi( const char *ssid, const char *pwd );
        //wifi event handler
        static void WiFiEvent( WiFiEvent_t event );

    public:
        explicit TransportWifiUDP( uint16_t port );
        ~TransportWifiUDP();

        uint16_t getPort() const;
        static bool isConnected() { return connected; }

        WiFiUDP &getUDP() const { return udp; }

        static TransportWifiUDP &singleton();
    };
#endif
}

#endif