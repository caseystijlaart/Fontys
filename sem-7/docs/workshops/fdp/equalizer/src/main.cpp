#include <TeensyThreads.h>

#include "audio.hpp"

void playSongThreadFunction() {
  Audio audio;
  audio.Init();
  while (1) {
    audio.PlaySong();
  }
}

void setup() {
  Audio audio;
  audio.Init();

  threads.addThread(playSongThreadFunction);
}

void loop() { delay(100); }
