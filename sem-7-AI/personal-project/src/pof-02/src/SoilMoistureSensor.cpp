#include "SoilMoistureSensor.hpp"

#include <Arduino.h>

#include <algorithm>

namespace pof02 {

SoilMoistureSensor::SoilMoistureSensor(int pin, float adcDry, float adcWet)
    : pin_(pin), adcDry_(adcDry), adcWet_(adcWet) {}

float SoilMoistureSensor::ReadPercent() const {
    const int raw = analogRead(pin_);
    const float scaled = (static_cast<float>(raw) - adcDry_) / (adcWet_ - adcDry_);
    return std::max(0.0f, std::min(100.0f, scaled * 100.0f));
}

} // namespace pof02
