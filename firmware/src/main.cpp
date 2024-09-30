#include <Arduino.h>

#include <sqid.h>
#include <sender.h>
#include <sampleFrame.h>

using namespace sqid;

const int DEVICE_ID = 10;
const int SENSOR_ID_BASE = 10;

const int ARRAY_SIZE  = 3;
const int MATRIX_SIZE_X = 4;
const int MATRIX_SIZE_Y = 5;

SampleFrame pointFrame(  DEVICE_ID, SENSOR_ID_BASE + 0, SampleFrame::L_POINT, SampleFrame::DT_FLOAT );
SampleFrame arrayFrame(  DEVICE_ID, SENSOR_ID_BASE + 1, SampleFrame::L_ARRAY, SampleFrame::DT_FLOAT, ARRAY_SIZE );
SampleFrame matrixFrame( DEVICE_ID, SENSOR_ID_BASE + 2, SampleFrame::L_MATRIX, SampleFrame::DT_FLOAT, MATRIX_SIZE_X, MATRIX_SIZE_Y );

SenderSerial pointSender;
SenderSerial arraySender;
SenderSerial matrixSender;

float t = 0.0f;
float speed = 1.0f;
uint8_t offset = 0;

void setup()
{
  //setup serial connection
  Serial.begin( 115200 );

  if( !pointSender.init( &pointFrame, MF_ENC_UNCOMPRESSED ) )
    Serial.println( "failed to init pointSender" );
  if( !arraySender.init( &arrayFrame, MF_ENC_UNCOMPRESSED ) )
    Serial.println( "failed to init arraySender" );
  if( !matrixSender.init( &matrixFrame, MF_ENC_UNCOMPRESSED ) )
    Serial.println( "failed to init matrixSender" );
}

void loop() {
  //dummy data
  t = t + speed;
  
  float *ptr = nullptr;

  //update point data
  ptr = reinterpret_cast<float*>( pointFrame.getData() );
  ptr[0] = ( (uint8_t)( offset + t ) % 256 ) / 255.0f;

  //update array data
  ptr = reinterpret_cast<float*>( arrayFrame.getData() );
  for (size_t i = 0; i < arrayFrame.getWidth(); i++ )
    ptr[i] = ( (uint8_t)(offset + t + i * 20 ) % 256 ) / 255.0f;

  //update matrix data
  for (size_t j = 0; j < matrixFrame.getHeight(); j++)
    for (size_t i = 0; i < matrixFrame.getHeight(); i++)
      ptr[i + (j * matrixFrame.getWidth())] = ( (uint8_t)(offset + t + (i + (j * matrixFrame.getWidth()) * 10)) % 256 ) / 255.0f;

  //send point data
  pointSender.send();

  //send array data
  arraySender.send();

  //send matrix data
  matrixSender.send();

  delay(10);
}
