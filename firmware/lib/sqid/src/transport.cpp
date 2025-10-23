/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#include "transport.h"

#include "Arduino.h"

namespace sqid
{
    TransportSerial::TransportSerial()
    {
        Serial.println( "sending via serial interface" );
    }

    TransportSerial::~TransportSerial()
    {
        Serial.end();
    }

    int TransportSerial::available()
    {
        return Serial.available();
    }

    size_t TransportSerial::readBytes( uint8_t *buffer, size_t length )
    {
        return Serial.readBytes( buffer, length );
    }

    size_t TransportSerial::write( const uint8_t *buffer, size_t size )
    {
        return Serial.write( buffer, size );
    }

    TransportSerial &TransportSerial::singleton()
    {
        static TransportSerial s;
        return s;
    }


#ifdef SUPPORT_BLUETOOTH_SERIAL
    TransportRFCOMM::TransportRFCOMM()
    {
        SerialBT.begin( "ESP32" ); //Name of your Bluetooth Signal
        Serial.println( "Bluetooth Device is Ready to Pair" );
    }

    TransportRFCOMM::~TransportRFCOMM()
    {
        SerialBT.end();
        Serial.println( "Bluetooth Device shut down" );
    }

    int TransportRFCOMM::available()
    {
        return SerialBT.available();
    }

    size_t TransportRFCOMM::readBytes( uint8_t *buffer, size_t length )
    {
        return SerialBT.readBytes( buffer, length );
    }

    size_t TransportRFCOMM::write( const uint8_t *buffer, size_t size )
    {
        size_t ret = SerialBT.write( buffer, size );
        if( SerialBT.getWriteError() )
        {
            Serial.println( "BT write error" );
            SerialBT.clearWriteError();
        }
        return ret;
    }

    static TransportRFCOMM &TransportRFCOMM::singleton()
    {
        static TransportRFCOMM rf;
        return rf;
    }
#endif


#ifdef SUPPORT_OSC
    WiFiUDP TransportWifiUDP::udp;
    bool TransportWifiUDP::connected = false;

    TransportWifiUDP::TransportWifiUDP( uint16_t port )
    {
        connectToWiFi( WIFI_SSID, WIFI_PWD );
    }

    TransportWifiUDP::~TransportWifiUDP()
    {
        udp.stop();
    }

    void TransportWifiUDP::connectToWiFi( const char *ssid, const char *pwd )
    {
        Serial.println( "Connecting to WiFi network: " + String( ssid ) );

        // delete old config
        WiFi.disconnect( true );
        connected = false;

        //register event handler
        WiFi.onEvent( WiFiEvent );
        
        //Initiate connection
        WiFi.begin( ssid, pwd );

        Serial.println( "Waiting for WIFI connection..." );
    }

    //wifi event handler
    void TransportWifiUDP::WiFiEvent( WiFiEvent_t event )
    {
        switch( event )
        {
        case SYSTEM_EVENT_STA_GOT_IP:
        {
            //When connected set 
            Serial.print( "WiFi connected! IP address: " );
            Serial.println( WiFi.localIP() );
            //initializes the UDP state
            //This initializes the transfer buffer
            if( !udp.begin( OUT_PORT ) )
            {
                Serial.print( "<error> setting up UDP no local port " );
                Serial.print( OUT_PORT );
                Serial.print( " failed\n" );
                connected = false;
            }
            else
                connected = true;
            break;
        }
        case SYSTEM_EVENT_STA_DISCONNECTED:
        {
            Serial.println( "WiFi lost connection" );
            connected = false;
            break;
        }
        default:
        {
            break;
        }
        }
    }

    uint16_t TransportWifiUDP::getPort() const
    {
        udp.remotePort();
    }

    TransportWifiUDP &TransportWifiUDP::singleton()
    {
        static TransportWifiUDP u( OUT_PORT );
        return u;
    }
#endif
}