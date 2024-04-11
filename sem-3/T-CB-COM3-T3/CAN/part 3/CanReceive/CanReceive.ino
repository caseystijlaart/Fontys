#include <SPI.h>
#include <mcp2515_can.h>

const int spiCSPin = 10;
const int LED      = 7;

mcp2515_can CAN(spiCSPin);

void setup() {
  Serial.begin(115200);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {
    Serial.println("CAN BUS init Failed");
    delay(100);
  }
  Serial.println("CAN BUS Shield Init OK!");
  pinMode(LED, OUTPUT);
}
void loop() {
  unsigned char len = 0;
  unsigned char buf[8];

  if (CAN_MSGAVAIL == CAN.checkReceive())
  {
    CAN.readMsgBuf(&len, buf);
    if ((buf[0] == 'O') && (buf[1] == 'N'))
    {
      digitalWrite(LED, HIGH);
      Serial.println("ON");
    }
    else if ((buf[0] == 'O') && (buf[1] == 'F') && (buf[2] == 'F'))
    {
      digitalWrite(LED, LOW);
      Serial.println("OFF");
    }
  }
}
