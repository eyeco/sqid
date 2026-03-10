#include <Arduino.h>

// #include <sqid.h>
#include <sender.h>
#include <sampleFrame.h>
#include <receiver.h>
#include <comMsgParser.h>

using namespace sqid;

// const int DEVICE_ID = 10;
// const int SENSOR_ID_BASE = 10;

// const int ARRAY_SIZE  = 3;
// const int MATRIX_SIZE_X = 4;
// const int MATRIX_SIZE_Y = 5;

// const int IMG_WIDTH  = 5;
// const int IMG_HEIGHT = 5;
// const int IMG_DEPTH = 3;

// SampleFrame pointFrame( DT_FLOAT );
// SampleFrame arrayFrame( DT_FLOAT, ARRAY_SIZE );
// SampleFrame matrixFrame( DT_FLOAT, MATRIX_SIZE_X, MATRIX_SIZE_Y );
// SampleFrame imgFrame( DT_FLOAT, IMG_WIDTH, IMG_HEIGHT, IMG_DEPTH );

// SenderSerial pointSender;
// SenderSerial arraySender;
// SenderSerial matrixSender;
// SenderSerial imgSender;

ComMsgParser parser;

SampleFrame errorFrame( DT_BYTE );
SampleFrame triggerFrame( DT_FLOAT );
SampleFrame sizeFrame( DT_FLOAT, 3 );

SenderSerial errorSender;
SenderSerial triggerSender;
SenderSerial sizeSender;

SenderSerial returnSender;

float t = 0.0f;
// float speed = 1.0f;
// float offset = 0.0f;

const size_t bufferSize = 1024;
char buffer[bufferSize];

ReceiverSerial receiver( 128 );

void setup()
{
  //setup serial connection
  Serial.begin( 115200 );

/*
  if( !pointSender.init( &pointFrame, DEVICE_ID, SENSOR_ID_BASE + 0 ) )
    Serial.println( "failed to init pointSender" );
  if( !arraySender.init( &arrayFrame, DEVICE_ID, SENSOR_ID_BASE + 1 ) )
    Serial.println( "failed to init arraySender" );
  if( !matrixSender.init( &matrixFrame, DEVICE_ID, SENSOR_ID_BASE + 2 ) )
    Serial.println( "failed to init matrixSender" );
  if( !imgSender.init( &imgFrame, DEVICE_ID, SENSOR_ID_BASE + 3 ) )
    Serial.println( "failed to init imgSender" );
*/

  if( !errorSender.init( &errorFrame, 1, 0 ) )
    Serial.println( "failed to init pointSender" );
  if( !triggerSender.init( &triggerFrame, 0, 0 ) )
    Serial.println( "failed to init pointSender" );
  if( !sizeSender.init( &sizeFrame, 0, 1 ) )
    Serial.println( "failed to init arraySender" );

}

void error( uint8_t errorCode )
{
    uint8_t *ptr = reinterpret_cast<uint8_t*>( errorFrame.getData() );
    ptr[0] = errorCode;

    errorSender.send();
}

void success()
{
    uint8_t *ptr = reinterpret_cast<uint8_t*>( errorFrame.getData() );
    ptr[0] = 0;

    errorSender.send();
}

void loop() {
  //dummy data
  //t = t + speed;
  
  float *ptr = nullptr;
  
  /*
  //update point data
  ptr = reinterpret_cast<float*>( pointFrame.getData() );
  ptr[0] = ( (uint8_t)( offset + t ) % 256 ) / 255.0f;

  //update array data
  ptr = reinterpret_cast<float*>( arrayFrame.getData() );
  for (size_t i = 0; i < arrayFrame.getWidth(); i++ )
    ptr[i] = ( (uint8_t)(offset + t + i * 20 ) % 256 ) / 255.0f;

  //update matrix data
  ptr = reinterpret_cast<float*>( matrixFrame.getData() );
  for (size_t j = 0; j < matrixFrame.getHeight(); j++)
    for (size_t i = 0; i < matrixFrame.getHeight(); i++)
      ptr[i + (j * matrixFrame.getWidth())] = ( (uint8_t)(t + (i + (j * matrixFrame.getWidth()) * 10)) % 256 ) / 255.0f;

  //update img data
  ptr = reinterpret_cast<float*>( imgFrame.getData() );
  for (size_t k = 0; k < imgFrame.getDepth(); k++)
    for (size_t j = 0; j < imgFrame.getHeight(); j++)
      for (size_t i = 0; i < imgFrame.getHeight(); i++)
        ptr[i + (j * imgFrame.getWidth()) + k * imgFrame.getWidth() * imgFrame.getHeight()] = ( (uint8_t)(offset + t + k * 500 + (i + (j * matrixFrame.getWidth()) * 10)) % 256 ) / 255.0f;

  //send point data
  pointSender.send();

  //send array data
  arraySender.send();

  //send matrix data
  matrixSender.send();

  //send matrix data
  imgSender.send();
  */

  /*
  int avail = Serial.available();
  while( avail )
  {
    if( avail > bufferSize)
      avail = bufferSize;
    size_t read = Serial.read(buffer, avail);

    Serial.write(buffer, read);
    //Serial.printf("available: %d", avail);

    avail = Serial.available();
  }
  */

  //update point data
  ptr = reinterpret_cast<float*>( triggerFrame.getData() );
  ptr[0] = 0;

  receiver.update();
  while( receiver.getMsgQueueSize() )
  {
    ComMsg msg = receiver.dequeue();

    DataType outType = DT_FLOAT;
    SampleFrame *sf = nullptr;

    switch( msg.hdr.hdr.type )
    {
    case MT_VALUE:
            sf = parser.createFrameFromSingleValue( msg.hdr.hdr, msg.data, outType );
            break;
    case MT_ARRAY:
            sf = parser.createFrameFromArray( msg.hdr.hdr, msg.data, outType );
            break;
    case MT_MATRIX:
            sf = parser.createFrameFromMatrix( msg.hdr.hdr, msg.data, outType );
            break;
    case MT_IMAGE:
            sf = parser.createFrameFromImage( msg.hdr.hdr, msg.data, outType );
            break;
    default:
          error( 1 );
            break;
    }
    if( !sf )
      error( 2 );
    else
    {
      size_t width = sf->getWidth();
      size_t height = sf->getHeight();
      size_t depth = sf->getDepth();

      ptr = reinterpret_cast<float*>( sizeFrame.getData() );
      ptr[0] = width;
      ptr[1] = height;
      ptr[2] = depth;

      sizeSender.send();

      if( !returnSender.init( sf, 2, 0 ) )
        error( 3 );
      else
        if( !returnSender.send() )
          error( 4 );

      success();
      safeDelete( sf );
    }

    ptr = reinterpret_cast<float*>( triggerFrame.getData() );
    ptr[0] = 1;

    triggerSender.send();

    safeDeleteArray( msg.data );
  }

  delay(10);
}
