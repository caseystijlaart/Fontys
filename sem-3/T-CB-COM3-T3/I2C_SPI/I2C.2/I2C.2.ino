#include <Wire.h>
#define slaveID (0xBE>>1)
#define lowRegister 0x28
#define highRegister 0x29
float lastHum = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  Wire.beginTransmission(slaveID);
  Wire.write(highRegister);
  Wire.endTransmission();
  Wire.requestFrom(slaveID, 1);
  byte high_Byte = Wire.read();

  Wire.beginTransmission(slaveID);
  Wire.write(lowRegister);
  Wire.endTransmission();
  Wire.requestFrom(slaveID, 1);
  byte low_Byte = Wire.read();
  
  float low_Hum = (int)low_Byte / 2;
  float high_Hum = (int)high_Byte / 2;
  float hum = (low_Hum + high_Hum) / 2;

  if (hum != lastHum)
  {
    Serial.println(hum);
    lastHum = hum;
  }
  
  
}
