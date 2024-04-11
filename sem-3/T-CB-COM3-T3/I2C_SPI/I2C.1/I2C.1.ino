#include <Wire.h>
#define slaveID (0x94 >> 1)
#define highRegister 0x00
#define lowRegister 0x02
float lastTemp = 0;

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

  float temp = ((int)low_Byte / 256.00) + (int)high_Byte;

  if (temp != lastTemp)
  {
    Serial.println(temp, 2);
    lastTemp = temp;
  }

}
