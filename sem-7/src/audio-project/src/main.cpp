// // Copyright 2023 Casey Stijlaart

#include <Arduino.h>
#include <Audio.h>
#include <Bounce.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Stepper.h>
#include <TeensyThreads.h>
#include <Wire.h>

#define USE_TEENSY3_OPTIMIZED_CODE

#define SDCARD_CS_PIN BUILTIN_SDCARD
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN 13

AudioPlaySdWav playSdWav1;      // xy=163,135
AudioMixer4 mixer1;             // xy=332,167
AudioEffectGranular granular1;  // xy=504,155
AudioOutputI2S i2s1;            // xy=664,185
AudioConnection patchCord1(playSdWav1, 0, mixer1, 0);
AudioConnection patchCord2(playSdWav1, 1, mixer1, 1);
AudioConnection patchCord3(mixer1, granular1);
AudioConnection patchCord4(granular1, 0, i2s1, 0);
AudioConnection patchCord5(granular1, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;  // xy=236,248

#define GRANULAR_MEMORY_SIZE 12800  // enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

const char* filename = "CHINCHILLA.WAV";

int pitch = 0;

#define STEPS 2038
Stepper stepper0(STEPS, 2, 4, 3, 5);
int target_pos = 0;
int temp_pos = 0;
bool direction = true;
int start_pos = 0;

int trigPin = 24;  // Trigger
int echoPin = 25;  // Echo
volatile bool to_close_flag = false;
volatile bool to_close_flag_2 = false;

int steps = 0;

void alterPitch(int pitch) {
  if (pitch > 1) {
    return;
  }

  if (pitch == 0) {
    sgtl5000_1.dacVolume(0.8);
    granular1.stop();
  } else {
    sgtl5000_1.dacVolume(1);
    granular1.beginPitchShift(0.3);
  }
}

void playMusic() {
  while (1) {
    if (playSdWav1.isPlaying() == false) {
      playSdWav1.play("CHINCHILLA.WAV");
      Serial.print("Playing song");
      delay(5);  // brief delay for the library read WAV info
    }
  }
}

void readDistance() {
  while (1) {
    // if (!to_close_flag) {
    // to_close_flag_2 = false;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    int16_t duration = pulseIn(echoPin, HIGH);
    int16_t distance = duration * 0.034 / 2;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance >= (int16_t)0 && distance <= (int16_t)40) {
      Serial.println("Start Get Away!!");
      to_close_flag = true;
      to_close_flag_2 = true;
      int steps_to_make = random(0, 40);
      if (direction) {
        Serial.print("dir true");
        target_pos = -steps_to_make;
      } else {
        Serial.print("dir false");
        target_pos = steps_to_make;
      }
      temp_pos = 0;
      start_pos = temp_pos;
    }
    threads.delay(500);  // Delay for 1 second
    // }
  }
}

void runMotor() {
  while (1) {
    if (to_close_flag) {
      Serial.println("run motor");
      stepper0.setSpeed(16);
      if (target_pos != temp_pos) {
        if (target_pos > temp_pos) {
          stepper0.step(temp_pos++);
        } else {
          stepper0.step(temp_pos--);
        }
      } else {
        if (target_pos > start_pos) {
          direction = false;
        } else {
          direction = true;
        }
        to_close_flag = false;
        to_close_flag_2 = false;
        if (direction) {
          direction = false;
        } else {
          direction = true;
        }
      }
      Serial.print("temp : ");
      Serial.println(temp_pos);
      Serial.print("target : ");
      Serial.println(target_pos);
    } else {
      alterPitch(0);
      stepper0.setSpeed(12);
      if (direction) {
        temp_pos++;
      } else {
        temp_pos--;
      }

      stepper0.step(temp_pos);
    }
  }
}

void setup() {
  Serial.begin(9600);

  AudioMemory(10);
  sgtl5000_1.enable();

  sgtl5000_1.volume(1);
  sgtl5000_1.dacVolume(0.8);
  sgtl5000_1.dacVolumeRampDisable();
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);

  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);

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

  delay(1000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  threads.addThread(playMusic);
  threads.addThread(readDistance);
  threads.addThread(runMotor);
}

void loop() {
  threads.delay(100);
  if (to_close_flag_2) {
    alterPitch(1);
  } else {
    alterPitch(0);
  }
}
