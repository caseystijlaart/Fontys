/* Copyright 2024 Casey Stijlaart*/

#include "audio.hpp"

Audio::Audio() : patchCord1(playSdWav1, 0, i2s1, 0) {}

void Audio::Init() {
  sgtl5000_1.enable();
  sgtl5000_1.volume(1);
  sgtl5000_1.dacVolume(0.8);
  sgtl5000_1.dacVolumeRampDisable();

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
    if (SD.exists(filename)) {
      Serial.println("File exists!");
    } else {
      Serial.println("File does not exist.");
    }
  }
  delay(500);
}

void Audio::PlaySong() {
  while (1) {
    if (playSdWav1.isPlaying() == false) {
      playSdWav1.play("CHINCHILLA.WAV");
      Serial.print("Playing song");
      delay(5);  // brief delay for the library read WAV info
    }
  }
}
