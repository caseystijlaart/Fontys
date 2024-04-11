/* Copyright 2024 Casey Stijlaart */

#ifndef DOCS_WORKSHOPS_FDP_EQUALIZER_SRC_AUDIO_HPP_
#define DOCS_WORKSHOPS_FDP_EQUALIZER_SRC_AUDIO_HPP_

#include <Arduino.h>
#include <Audio.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>

#define SDCARD_CS_PIN BUILTIN_SDCARD
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN 13

class Audio {
 private:
  AudioPlaySdWav playSdWav1;
  AudioOutputI2S i2s1;
  AudioConnection patchCord1;  // Moved the declaration here
  AudioControlSGTL5000 sgtl5000_1;

  const char* filename = "CHINCHILLA.WAV";

 public:
  Audio();  // Constructor added
  void Init();
  void PlaySong();
};
#endif  // DOCS_WORKSHOPS_FDP_EQUALIZER_SRC_AUDIO_HPP_
