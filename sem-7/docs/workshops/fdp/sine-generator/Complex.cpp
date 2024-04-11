/* Copyright 2024 Casey Stijlaart */

#include "Complex.hpp"

Complex::Complex(double real, double imag) : re(real), im(imag) {}

Complex Complex::operator*(const Complex& other) const {
    Complex result(0.0, 0.0);
    result.re = re * other.re - im * other.im;
    result.im = re * other.im + im * other.re;
    return result;
}