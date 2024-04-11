/* Copyright 2024 Casey Stijlaart */

#define _USE_MATH_DEFINES

#include <cmath>

#include "../sine-generator/Complex.hpp"

int main() {
  Complex* z = new Complex(cos(2 * M_PI / 128), sin(2 * M_PI / 128));
  Complex* y = new Complex(cos(2 * M_PI / 64), sin(2 * M_PI / 64));
}
