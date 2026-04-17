#include "LightSensor.hpp"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <algorithm>
#include <cmath>
#include <ctime>
#endif

namespace pof02 {

LightSensor::LightSensor(int pin) : pin_(pin) {}

float LightSensor::ReadLux() const {
#ifdef ARDUINO
    const int raw = analogRead(pin_);
    return (static_cast<float>(raw) / 4095.0f) * 1200.0f;
#else
    const double t = static_cast<double>(std::time(nullptr));
    const double daylight = 450.0 + 350.0 * std::max(0.0, std::sin(t / 3600.0));
    return static_cast<float>(daylight);
#endif
}

} // namespace pof02
