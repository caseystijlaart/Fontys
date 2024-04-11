#include <Wire.h>
#include <SPI.h>

char incoming;

void setup() {
  Serial.begin(115200);
  digitalWrite(SS, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void loop() {
  if (Serial.available() > 0)
  {
    incoming = Serial.read();
    if ((incoming == 'a') || (incoming == 'b'))
    {
      SPISend(incoming);
    }
  }
}

void SPISend(char value) {
  digitalWrite(SS, LOW);
  SPI.transfer(value);
  digitalWrite(SS, HIGH);
}
