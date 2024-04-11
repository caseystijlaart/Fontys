#include <Wire.h>
#define slavePressID (0xBA>>1)
#define PRESS_OUT_H 0x2A
#define PRESS_OUT_L 0x29
#define PRESS_OUT_XL 0x28

#define slaveTempID (0x94 >> 1)
#define highTempRegister 0x00
#define lowTempRegister 0x02

#define slaveHumID (0xBE >> 1)
#define highHumRegister 0x28
#define lowHumRegister 0x29

#define slaveID 0x20
#define horiRegister 0x03
#define vertRegister 0x05
int lastHorVal = 0;
int lastVerVal = 0;

void setup() {
  pinMode(13, HIGH);
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  Wire.beginTransmission(slaveID);
  Wire.write(horiRegister);
  Wire.endTransmission();
  Wire.requestFrom(slaveID, 1);
  byte horiValue = Wire.read();
  int horVal = map(horiValue , 0, 256, -10, 10);

  Wire.beginTransmission(slaveID);
  Wire.write(vertRegister);
  Wire.endTransmission();
  Wire.requestFrom(slaveID, 1);
  byte vertiValue = Wire.read();
  int verVal = map(vertiValue , 0, 256, -10, 10);

  if ((horVal != lastHorVal) || (verVal != lastVerVal))
  {
    if ((horVal == -1) && (verVal == 9))
    {
      getTemp();
      digitalWrite(13, LOW);
    }
    else if ((horVal == -1) && (verVal == -10))
    {
      getHum();
      digitalWrite(13, LOW);
    }
    if ((horVal == -10) && (verVal == 0))
    {
      digitalWrite(13, HIGH);
    }
    if ((horVal == 9) && (verVal == 0))
    {
      getTemp();
      getHum();
      digitalWrite(13, LOW);
    }

    lastHorVal = horVal;
    lastVerVal = verVal;
  }
}


void getTemp()
{
  Wire.beginTransmission(slaveTempID);
  Wire.write(highTempRegister);
  Wire.endTransmission();
  Wire.requestFrom(slaveTempID, 1);
  byte high_Byte = Wire.read();

  Wire.beginTransmission(slaveTempID);
  Wire.write(lowTempRegister);
  Wire.endTransmission();
  Wire.requestFrom(slaveTempID, 1);
  byte low_Byte = Wire.read();

  float temp = ((int)low_Byte / 256.00) + (int)high_Byte;
  Serial.print("Temperature : ");
  Serial.println(temp);
}

void getHum()
{
  Wire.beginTransmission(slaveHumID);
  Wire.write(highHumRegister);
  Wire.endTransmission();
  Wire.requestFrom(slaveHumID, 1);
  byte high_Byte = Wire.read();

  Wire.beginTransmission(slaveHumID);
  Wire.write(lowHumRegister);
  Wire.endTransmission();
  Wire.requestFrom(slaveHumID, 1);
  byte low_Byte = Wire.read();

  float hum = ((((int)low_Byte / 2) + ((int)high_Byte / 2)) / 2);
  Serial.print("Humidity : ");
  Serial.println(hum);
}
