#include<SPI.h>
byte Slavereceived;
boolean received;
const int LED0 = 7;

void setup() {
  Serial.begin(115200);
  pinMode(LED0, OUTPUT);
  SPCR |= bit (SPE);
  pinMode(MISO, OUTPUT);
  received = false;
  SPI.attachInterrupt();
}

ISR (SPI_STC_vect)                      
{
  Slavereceived = SPDR;        
  received = true;            
}

void loop() {
  if (received)
  {
    if (Slavereceived == 'a')
    {
      digitalWrite(LED0, HIGH);
      Serial.println("on");
    } 
    if (Slavereceived == 'b')
    {
      digitalWrite(LED0, LOW);
      Serial.println("off");
    }
    received = false;
  }
}
