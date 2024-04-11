#include <SPI.h>
#include <mcp2515_can.h>

const int spiCSPin = 10;
mcp2515_can CAN(spiCSPin);

unsigned char stmp[8] = {};
int mesID = 0x01;

void setup()
{
  Serial.begin(115200);
  while (CAN_OK != CAN.begin(CAN_500KBPS))
  {
    Serial.println("CAN BUS init Failed");
    delay(100);
  }
  Serial.println("CAN BUS Shield Init OK!");
}

void loop()
{
  if (Serial.available() > 0)
  {
    String str = Serial.readString();
    if (str == "ON")
    {
      stmp[0] = 'O';
      stmp[1] = 'N';
      CAN.sendMsgBuf(mesID, 0, sizeof(stmp), stmp);
    } else if (str == "OFF")
    {
      stmp[0] = 'O';
      stmp[1] = 'F';
      stmp[2] = 'F';
      CAN.sendMsgBuf(mesID, 0, sizeof(stmp), stmp);
    }

    // to clear the the array after sending
    for (int i = 0; i < sizeof(stmp); i++)
    {
      stmp[i] = {};
    }
  }
}
