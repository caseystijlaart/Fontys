#include "SoilMoistureSensor.hpp"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <algorithm>
#include <cmath>
#include <ctime>
#endif

namespace pof02 {

SoilMoistureSensor::SoilMoistureSensor(int pin, float adcDry, float adcWet)
    : pin_(pin), adcDry_(adcDry), adcWet_(adcWet) {}

float SoilMoistureSensor::ReadPercent() const {
#ifdef ARDUINO
    const int raw = analogRead(pin_);
    const float scaled = (static_cast<float>(raw) - adcDry_) / (adcWet_ - adcDry_);
    return std::max(0.0f, std::min(100.0f, scaled * 100.0f));
#else
    const double t = static_cast<double>(std::time(nullptr));
    const double v = 45.0 + 20.0 * std::sin(t / 900.0);
    return static_cast<float>(std::max(0.0, std::min(100.0, v)));
#endif
}

} // namespace pof02
