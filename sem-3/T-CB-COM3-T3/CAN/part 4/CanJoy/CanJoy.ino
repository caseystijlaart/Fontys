#include <SPI.h>
#include <mcp2515_can.h>
#include <Wire.h>
#include "SparkFun_Qwiic_Joystick_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_joystick
JOYSTICK joystick;

const int spiCSPin = 10;
int mesIdA = 0x04;
int mesIdB = 0x05;

mcp2515_can CAN(spiCSPin);

unsigned char stmp[8] = {};

void setup() {
  Serial.begin(115200);
  while (CAN_OK != CAN.begin(CAN_500KBPS))
  {
    Serial.println("CAN BUS init Failed");
    delay(100);
  }
  Serial.println("CAN BUS Shield Init OK!");
  joystick.begin();
}

void loop() {
  int xMap = map(joystick.getHorizontal(), 0, 1023, -5, 5);
  int yMap = map(joystick.getVertical(), 0, 1023, -5, 5);
  
  if (((xMap == -1) && (yMap == -5)))
  {
    stmp[0] = 'L';
    stmp[1] = 'E';
    stmp[2] = 'F';
    stmp[3] = 'T';
    CAN.sendMsgBuf(mesIdA, 0, sizeof(stmp), stmp);
  } else if ((xMap == -1) && (yMap == 5))
  {
    stmp[0] = 'R';
    stmp[1] = 'I';
    stmp[2] = 'G';
    stmp[3] = 'H';
    stmp[4] = 'T';
    CAN.sendMsgBuf(mesIdA, 0, sizeof(stmp), stmp);
  } 
  else if ((xMap == 5) && (yMap == 0))
  {
    stmp[0] = 'O';
    stmp[1] = 'N';
    CAN.sendMsgBuf(mesIdB, 0, sizeof(stmp), stmp);
  } else if ((xMap == -5) && (yMap == 0))
  {
    stmp[0] = 'O';
    stmp[1] = 'F';
    stmp[2] = 'F';
    CAN.sendMsgBuf(mesIdB, 0, sizeof(stmp), stmp);
  }else if ((xMap = -1) && (yMap == 0))
  {
    stmp[0] = 'M';
    stmp[1] = 'I';
    stmp[2] = 'D';
    CAN.sendMsgBuf(mesIdA, 0, sizeof(stmp), stmp);
  }

  for (int i = 0; i < sizeof(stmp); i++)
  {
    stmp[i] = {};
  }
  delay(500);
}
