#include "sender.h"

#include "sampleFrame.h"

//#include "rle.h"
#include "compressor.h"
#include "transport.h"

#include "Arduino.h"

namespace sqid
{
    size_t getElementSize( MsgFlags flags )
    {
        switch( MF_DATATYPE( flags ) )
        {
          case MF_DATATYPE_BYTE:
              return sizeof( uint8_t );
          case MF_DATATYPE_INT16:
              return sizeof( uint16_t );
          case MF_DATATYPE_INT32:
              return sizeof( uint32_t );
          case MF_DATATYPE_FLOAT:
              return sizeof( float );
          default:
              break;
        }
        
        return 0;
    }

    SenderSerial::SenderSerial() :
        Sender(),
        _flags( MF_NONE ),
        _err( false ),
        _inputDataBytes( 0 ),
        _headerBytes( 0 ),
        _payloadBytesMax( 0 ),
        _frame( nullptr ),
        _compressor( nullptr )
    {
        _comMsg.hdr.hdr.ver = PV_2;
        _comMsg.hdr.hdr.timeStamp = 0;
    }

    SenderSerial::~SenderSerial()
    {
        _frame = nullptr;

        safeDelete( _compressor );
        safeDeleteArray( _comMsg.data );
    }

    bool SenderSerial::init( const SampleFrame *frame, unsigned char deviceID, unsigned char sensorID, MsgFlags encoding )
    {
        _flags = MF_NONE;
        _frame = frame;

        MsgType messageType = MT_COUNT;
        switch( _frame->getLayout() ) 
        {
        case L_0D:
        {
            messageType = MT_VALUE;
            break;
        }
        case L_1D:
        {
            messageType = MT_ARRAY;
            break;
        }
        case L_2D:
        {
            messageType = MT_MATRIX;
            break;
        }
        case L_3D:
        {
            messageType = MT_IMAGE;
            break;
        }
        default:
        {
            // unknown layout
            _err = true;
            return false;
        }
        }

        switch( _frame->getDataType() )
        {
        case DT_BYTE:
        {
            _flags = (MsgFlags)( (int)_flags | (int)MF_DATATYPE_BYTE );
            break;
        }
        case DT_USHORT:
        {
            _flags = (MsgFlags)( (int)_flags | (int)MF_DATATYPE_INT16 );
            break;
        }
        case DT_ULONG:
        {
            _flags = (MsgFlags)( (int)_flags | (int)MF_DATATYPE_INT32 );
            break;
        }
        case DT_FLOAT:
        {
            _flags = (MsgFlags)( (int)_flags | (int)MF_DATATYPE_FLOAT );
            break;
        }
        default:
        {
            _err = true;
            return false;
        }
        }

        size_t width = _frame->getWidth();
        size_t height = _frame->getHeight();
        size_t depth = _frame->getDepth();

        _inputDataBytes = width * height * depth * getElementSize( _flags );

        if( !_inputDataBytes )
        {
            //TODO: print error or something? assert? throw an exception? what's the correct thing to do in arduino code?
            _err = true;
            return false;
        }
        //TODO: also doublecheck flags argument

        safeDelete( _compressor );
        switch( MF_ENCODING( encoding ) )
        {
        case MF_ENC_UNCOMPRESSED:
        {
            _compressor = new CompressorDummy();
            _flags = (MsgFlags)( (int)_flags | (int)MF_ENC_UNCOMPRESSED );
            break;
        }
        case MF_ENC_RLE:
        {
#ifdef _MIL_RLE_ENC_SUPPORT
            _compressor = new CompressorRLE();
            _flags = (MsgFlags)( (int)_flags | (int)MF_ENC_RLE );
#endif
            break;
        }
        case MF_ENC_LZO:
        {
#ifdef _MIL_LZO_ENC_SUPPORT
            _compressor = new CompressorLZO();
            _flags = (MsgFlags)( (int)_flags | (int)MF_ENC_LZO );
#endif
            break;
        }
        case MF_ENC_ZSTD: 
        {
#ifdef _MIL_ZSTD_ENC_SUPPORT
            _compressor = new CompressorZStd();
            _flags = (MsgFlags)( (int)_flags | (int)MF_ENC_ZSTD );
#endif
            break;
        }        
        case MF_ENC_LZ4:
        {
#ifdef _MIL_LZ4_ENC_SUPPORT
            _compressor = new CompressorLZ4();
            _flags = (MsgFlags)( (int)_flags | (int)MF_ENC_LZ4 );
#endif
            break;
        }
        case MF_ENC_ZLIB:
        default:
        {
            //type not implemented
            break;
        }
        }

        if( !_compressor )
        {
            _err = true;
            return false;
        }

        //TODO: switch to sending raw data in case block is not compressible
        _payloadBytesMax = _compressor->getMinOutBufferSize( _inputDataBytes );

        if(messageType == MsgType::MT_VALUE)
        {
            //calculate required data array size for a single frame value and set it 
            size_t dataBytesMax = _payloadBytesMax + sizeof(DataHdrValue);
            _comMsg.data = new unsigned char[dataBytesMax];

            //create value header to set attributes
            DataHdrValue *dataHdrPtr = (DataHdrValue*) _comMsg.data;
            dataHdrPtr->flags = _flags;

            //store header size for later use (data object pointer offset)
            _headerBytes = sizeof(DataHdrValue);
        }
        else if(messageType == MsgType::MT_ARRAY)
        {
            //calculate required data array size for array frame and set it 
            size_t dataBytesMax = _payloadBytesMax + sizeof(DataHdrArray);
            _comMsg.data = new unsigned char[dataBytesMax];

            //create array header to set attributes
            DataHdrArray *dataArrayHdrPtr = (DataHdrArray*) _comMsg.data;
            dataArrayHdrPtr->flags = _flags;
            dataArrayHdrPtr->size = width;

            //store header size for later use (data object pointer offset)
            _headerBytes = sizeof(DataHdrArray);      
        }
        else if(messageType == MT_MATRIX)
        {
            //calculate required data array size for a matrix frame and set it 
            size_t dataBytesMax = _payloadBytesMax + sizeof(DataHdrMatrix);
            _comMsg.data = new unsigned char[dataBytesMax];

            //create matrix header to set attributes
            DataHdrMatrix *dataMatrixHdrPtr = (DataHdrMatrix*) _comMsg.data;
            dataMatrixHdrPtr->flags = _flags;
            dataMatrixHdrPtr->width = width;
            dataMatrixHdrPtr->height = height;

            //store header size for later use (data object pointer offset)
            _headerBytes = sizeof(DataHdrMatrix);
        }
        else if(messageType == MT_IMAGE)
        {
            //calculate required data array size for an image frame and set it 
            size_t dataBytesMax = _payloadBytesMax + sizeof(DataHdrImage);
            _comMsg.data = new unsigned char[dataBytesMax];

            //create image header to set attributes
            DataHdrImage *dataMatrixHdrPtr = (DataHdrImage*) _comMsg.data;
            dataMatrixHdrPtr->flags = _flags;
            dataMatrixHdrPtr->width = width;
            dataMatrixHdrPtr->height = height;
            dataMatrixHdrPtr->depth = depth;

            //store header size for later use (data object pointer offset)
            _headerBytes = sizeof(DataHdrImage);
        }
        else
        {
            //TODO: print error or something? assert? throw an exception? what's the correct thing to do in arduino code?
             _err = true;
            return false;
        }

        _comMsg.hdr.hdr.deviceID = deviceID;
        _comMsg.hdr.hdr.sensorID = sensorID;

        //set header type to single value for the receiver to to able to identify the data type
        _comMsg.hdr.hdr.type = messageType;

        return true;
    }

    bool SenderSerial::send()
    {
        if( _err )
            return false;

        //set timestamp
        unsigned long time = millis();
        _comMsg.hdr.hdr.timeStamp = time;

        uint16_t payloadBytes = 0;
        unsigned char *payloadPtr = (unsigned char*)_comMsg.data + _headerBytes;

        if( !_compressor )
        {
            _err = true;
            return false;
        }

        MsgFlags enc = MF_ENCODING( _flags );

        const uint8_t *data = _frame->getData();
        if( !data )
        {
            _err = true;
            return false;
        }

        payloadBytes = _compressor->compress( data, _inputDataBytes, payloadPtr, _payloadBytesMax );

        if( !payloadBytes )
        {
            _err = true;
            return false;
        }


        //if block is incompressible, send raw data
        if( payloadBytes > _inputDataBytes )
        {
            enc = MF_ENC_UNCOMPRESSED;
            payloadBytes = _inputDataBytes;
            memcpy( payloadPtr, data, payloadBytes );
        }

        switch( _comMsg.hdr.hdr.type )
        {
        case MT_VALUE:
        {
            DataHdrValue *hdr = (DataHdrValue*)_comMsg.data;
            hdr->flags = (MsgFlags)( ( (int)hdr->flags & ~MF_ENC_MASK ) | (int)enc );
            break;
        }
        case MT_ARRAY:
        {
            DataHdrArray *hdr = (DataHdrArray*)_comMsg.data;
            hdr->flags = (MsgFlags)( ( (int)hdr->flags & ~MF_ENC_MASK ) | (int)enc );
            break;
        }
        case MT_MATRIX:
        {
            DataHdrMatrix *hdr = (DataHdrMatrix*)_comMsg.data;
            hdr->flags = (MsgFlags)( ( (int)hdr->flags & ~MF_ENC_MASK ) | (int)enc );
            break;
        }
        case MT_IMAGE:
        {
            DataHdrImage *hdr = (DataHdrImage*)_comMsg.data;
            hdr->flags = (MsgFlags)( ( (int)hdr->flags & ~MF_ENC_MASK ) | (int)enc );
            break;
        }
        default:
        {
            _err = true;
            return false;
        }
        }

        uint16_t dataBytes = payloadBytes + _headerBytes;

        _comMsg.hdr.hdr.dataBytes = dataBytes;

        //update checksum
        _comMsg.hdr.chk = makeChkSum( &_comMsg.hdr.hdr );

        //send sync bytes
        write( (uint8_t*)syncBytes, SYNC_BYTES );

        //send header
        write( (uint8_t*)&( _comMsg.hdr ), (int) sizeof( ComMsgHdrEx ) );

        //send data
        write( (uint8_t*)_comMsg.data, dataBytes );

        return true;
    }

    bool SenderSerial::sendFWPackage( MsgType type, const char *data, size_t len )
    {
        if( !data )
           return false;

        ComMsg paramMsg;
        paramMsg.hdr.hdr.ver = _comMsg.hdr.hdr.ver;
        paramMsg.hdr.hdr.type = type;
        paramMsg.hdr.hdr.deviceID = _comMsg.hdr.hdr.deviceID;
        paramMsg.hdr.hdr.sensorID = _comMsg.hdr.hdr.sensorID;
        paramMsg.hdr.hdr.timeStamp = millis();
        paramMsg.hdr.hdr.dataBytes = len;

        paramMsg.hdr.chk = makeChkSum( &paramMsg.hdr.hdr );

        paramMsg.data = (uint8_t*)data;

        //send sync bytes
        write( (uint8_t*)syncBytes, SYNC_BYTES );

        //send header
        write( (uint8_t*)&( paramMsg.hdr ), (int) sizeof( ComMsgHdrEx ) );

        //send data
        write( (uint8_t*)paramMsg.data, paramMsg.hdr.hdr.dataBytes);

        return true;
    }

    bool SenderSerial::sendFWDesc( const char *desc )
    {
        if( !desc )
            return false;

        return sendFWPackage( MT_FWPROPS_DESC, desc, strlen( desc ) );
    }

    bool SenderSerial::sendFWStatus( const char *status )
    {
        if( !status )
            return false;

        return sendFWPackage( MT_FWPROPS_STATUS, status, strlen( status ) );
    }

    bool SenderSerial::write( const uint8_t *buffer, size_t size )
    {
        return ( TransportSerial::singleton().write( buffer, size ) == size );
    }

    int SenderSerial::available()
    {
        return TransportSerial::singleton().available();
    }

    int SenderSerial::readBytes( uint8_t *data, size_t bytes )
    {
        return TransportSerial::singleton().readBytes( data, bytes );
    }

    bool SenderSerial::sendFWAck( const char *status )
    {
         if( !status )
            return false;

        return sendFWPackage( MT_FWPROPS_ACK, status, strlen( status ) );
    }



#ifdef SUPPORT_BLUETOOTH_SERIAL
    SenderRFCOMM::SenderRFCOMM() :
        SenderSerial()
    {}

    SenderRFCOMM::~SenderRFCOMM()
    {}

    bool SenderRFCOMM::write( const uint8_t *buffer, size_t size )
    {
        return ( TransportRFCOMM::singleton().write( buffer, size ) == size );
    }

    int SenderRFCOMM::available()
    {
        return TransportRFCOMM::singleton().available();
    }

    int SenderRFCOMM::readBytes( uint8_t *data, size_t bytes )
    {
        return TransportRFCOMM::singleton().readBytes( data, bytes );
    }
#endif



#ifdef SUPPORT_OSC
    SenderOSC::SenderOSC( const char *address, uint16_t port ) :
        Sender(),
        _port( port ),
        _frame( nullptr )
    {
        _ip.fromString( address );
    }

    SenderOSC::~SenderOSC()
    {}

    bool SenderOSC::init( const SampleFrame *frame, MsgFlags encoding )
    {
        _frame = frame;

        //trigger connection by accessing/creating singleton;
        TransportWifiUDP::singleton();

        //encoding/compression not yet supported
        if( encoding != MF_ENC_UNCOMPRESSED )
            return false;

        return true;
    }

    bool SenderOSC::send()
    {
        if( !_frame )
            return false;

        if( !TransportWifiUDP::isConnected() )
            return false;

        int ts = millis();

        OSCBundle bndl;

        OSCMessage &msg = bndl.add( "/frame" );
        msg.add( (int32_t)_frame->getDeviceID() );
        msg.add( (int32_t)_frame->getSensorID() );
        msg.add( (int32_t)_frame->getWidth() );
        msg.add( (int32_t)_frame->getHeight() );
        msg.add( (int32_t)_frame->getDepth() );
        msg.add( (int32_t)ts );

        msg.add( _frame->getData(), _frame->getSize() );

        WiFiUDP &udp = TransportWifiUDP::singleton().getUDP();

        if( !udp.beginPacket( _ip, _port ) )
            return false;

        bndl.setTimetag( oscTime() );
        bndl.send( udp ); // send the bytes to the SLIP stream

        if( !udp.endPacket() ) // mark the end of the OSC Packet
            return false;

        return true;
    }
#endif

}
