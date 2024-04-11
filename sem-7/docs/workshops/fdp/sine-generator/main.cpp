/* Copyright 2024 Casey Stijlaart */

#include <cmath>
#include <fstream>
#include <iostream>

#include "Complex.hpp"  // NOLINT

int main() {
  const double frequencyStart = 0;
  const double frequencyEnd = 10;
  const double frequencyStep = 0.01;

  Complex* gen = new Complex(1.0, 0.0);
  std::ofstream outputFile("output.txt");

  for (double frequency = frequencyStart; frequency <= frequencyEnd;
       frequency += frequencyStep) {
    Complex* z = new Complex(cos(frequency), sin(frequency));

    *gen = (*gen) * (*z);

    std::cout << "Frequency: " << frequency << "\tGen.re: " << gen->re
              << "\tGen.im: " << gen->im << std::endl;

    outputFile << frequency << "\t" << gen->re << "\t" << gen->im << std::endl;

    delete z;
  }

  delete gen;

  return 0;
}