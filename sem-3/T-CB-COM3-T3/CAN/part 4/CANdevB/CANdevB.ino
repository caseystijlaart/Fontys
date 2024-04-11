#include <SPI.h>
#include <mcp2515_can.h>

const int spiCSPin = 10;
const int LED0      = 7;
int mesId = 0x05;

mcp2515_can CAN(spiCSPin);

void setup() {
  Serial.begin(115200);
  pinMode(LED0, OUTPUT);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {
    Serial.println("CAN BUS init Failed");
    delay(100);
  }
  Serial.println("CAN BUS Shield Init OK!");

  CAN.init_Mask(0, 0, 0x3ff);                         
  CAN.init_Mask(1, 0, 0x3ff);

  // initialise all the 6 filter to the specific address 
  CAN.init_Filt(0, 0, mesId); 
  CAN.init_Filt(1, 0, mesId);
  CAN.init_Filt(2, 0, mesId);
  CAN.init_Filt(3, 0, mesId);                          
  CAN.init_Filt(4, 0, mesId);                          
  CAN.init_Filt(5, 0, mesId);                         
}

void loop() {
  unsigned char len = 0;
  unsigned char buf[8];
  
  while (CAN_MSGAVAIL == CAN.checkReceive())
  {
    CAN.readMsgBuf(&len, buf);
    if ((buf[0] == 'O') && (buf[1] == 'N'))
    {
      digitalWrite(LED0, HIGH);
    }
    else if ((buf[0] == 'O') && (buf[1] == 'F') && (buf[2] == 'F'))
    {
      digitalWrite(LED0, LOW);
    }
  }
}
