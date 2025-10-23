/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

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

const int IMG_WIDTH  = 5;
const int IMG_HEIGHT = 5;
const int IMG_DEPTH = 3;

SampleFrame pointFrame( DT_FLOAT );
SampleFrame arrayFrame( DT_FLOAT, ARRAY_SIZE );
SampleFrame matrixFrame( DT_FLOAT, MATRIX_SIZE_X, MATRIX_SIZE_Y );
SampleFrame imgFrame( DT_FLOAT, IMG_WIDTH, IMG_HEIGHT, IMG_DEPTH );

SenderSerial pointSender;
SenderSerial arraySender;
SenderSerial matrixSender;
SenderSerial imgSender;

float t = 0.0f;
float speed = 1.0f;
float offset = 0.0f;

void setup()
{
  //setup serial connection
  Serial.begin( 115200 );

  if( !pointSender.init( &pointFrame, DEVICE_ID, SENSOR_ID_BASE + 0 ) )
    Serial.println( "failed to init pointSender" );
  if( !arraySender.init( &arrayFrame, DEVICE_ID, SENSOR_ID_BASE + 1 ) )
    Serial.println( "failed to init arraySender" );
  if( !matrixSender.init( &matrixFrame, DEVICE_ID, SENSOR_ID_BASE + 2 ) )
    Serial.println( "failed to init matrixSender" );
  if( !imgSender.init( &imgFrame, DEVICE_ID, SENSOR_ID_BASE + 3 ) )
    Serial.println( "failed to init imgSender" );
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

  delay(10);
}
