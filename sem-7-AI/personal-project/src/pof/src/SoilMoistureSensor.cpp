#include "SoilMoistureSensor.hpp"

SoilMoistureSensor::SoilMoistureSensor(uint8_t sensorPin, int dry, int wet)
    : pin(sensorPin), dryValue(dry), wetValue(wet) {}

bool SoilMoistureSensor::Init() {
    pinMode(pin, INPUT);
    return true;
}

float SoilMoistureSensor::GetData() {
    const int samples = 10;
    long total = 0;

    for (int i = 0; i < samples; i++) {
        total += analogRead(pin);
        delay(5);
    }

    float raw = total / static_cast<float>(samples);

    float percent = 100.0f * (dryValue - raw) / (dryValue - wetValue);

    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    float soilMoisture = 10.0f + (percent / 100.0f) * 30.0f;
    return soilMoisture;
}

SensorType SoilMoistureSensor::GetType() const {
    return SensorType::SOIL_MOISTURE;
}