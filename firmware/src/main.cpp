#include <Arduino.h>

#include <sqid.h>

#include <comMsg.h>
#include <sender.h>
#include <sampleFrame.h>

using namespace Sqid;

const int DEVICE_ID = 0;
const int SENSOR_ID = 0;

const int ANALOG_IN_COUNT = 6;

const int ANALOG_INS[] = {
  A0, A1, A2, A3, A4, A5
};

const int VCCS[] = {
  A6, A7, A8, A9, A10, A11
};

#define ADC_MAX     ((0x01 << 12) - 1)

uint32_t R_ref = 150;
float V_ref = 3.268;


SampleFrame frame( DEVICE_ID, SENSOR_ID, SampleFrame::L_ARRAY, SampleFrame::DT_FLOAT, ANALOG_IN_COUNT );
SenderSerial sender;

void setup() {
  Serial.begin( 115200 );

  if( !sender.init( &frame ) )
    Serial.println( "failed to init pointSender" );

  for(int i = 0; i < ANALOG_IN_COUNT; i++) {
    pinMode(ANALOG_INS[i], INPUT);
    pinMode(VCCS[i], INPUT);
  }
}

float multiSample(int id, int msT)
{
  static float s = 1.0f / ADC_MAX;
  unsigned long startMS = millis();

  pinMode(VCCS[id], OUTPUT);
  digitalWrite(VCCS[id], HIGH);
  delayMicroseconds(100);

  int cntr = 0;
  uint32_t sum = 0;
  do{
    sum += analogRead(ANALOG_INS[id]);
    cntr++;
  }while(millis() - startMS < msT);

  pinMode(VCCS[id], INPUT);

  return (float)sum / cntr * s;
}

void loop() {
  float f[ANALOG_IN_COUNT];
  for(int i = 0; i < ANALOG_IN_COUNT; i++)
    f[i] = multiSample(i, 3);

  if(!frame.setData(reinterpret_cast<const uint8_t*>(f))){
    Serial.println("setting data failed");
    delay(5);
  } else {
    sender.send();
  }
}
