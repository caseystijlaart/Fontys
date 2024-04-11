/* Copyright 2024 Casey Stijlaart */

#ifndef DOCS_WORKSHOPS_FDP_FREQUENCY_ANALYSER_FFT_HPP_
#define DOCS_WORKSHOPS_FDP_FREQUENCY_ANALYSER_FFT_HPP_

#include <stdint.h>

class FFT {
 public:
  uint16_t CalcSpectrium(uint8_t samples, uint8_t size);
};

#endif  //  DOCS_WORKSHOPS_FDP_FREQUENCY_ANALYSER_FFT_HPP_
