/* Copyright 2024 Casey Stijlaart */

#ifndef DOCS_WORKSHOPS_FDP_SINE_GENERATOR_COMPLEX_HPP_
#define DOCS_WORKSHOPS_FDP_SINE_GENERATOR_COMPLEX_HPP_

#include <cmath>
#include <iostream>

class Complex {
 public:
  double re;
  double im;

  Complex(double real, double imagine);

  Complex operator*(const Complex& other) const;
};

#endif  // DOCS_WORKSHOPS_FDP_SINE_GENERATOR_COMPLEX_HPP_
