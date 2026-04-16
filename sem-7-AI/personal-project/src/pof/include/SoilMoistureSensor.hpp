#pragma once

#include <Arduino.h>
#include "ISensor.hpp"

class SoilMoistureSensor : public ISensor
{
private:
    /* Pin number for the soil moisture sensor and calibration values for dry and wet soil */
    uint8_t pin;
    int dryValue;
    int wetValue;

public:
    /** @brief Constructor for SoilMoistureSensor
     * @param sensorPin Pin number for the soil moisture sensor
     * @param dry Dry soil calibration value
     * @param wet Wet soil calibration value
     */
    SoilMoistureSensor(uint8_t sensorPin, int dry, int wet);

    /** @brief Initialize the soil moisture sensor
     * @return true if initialization was successful, false otherwise
     */
    bool Init() override;

    /** @brief Get the soil moisture sensor data
     * @return The soil moisture percentage (0-100%)
     */
    float GetData() override;

    /** @brief Get the type of sensor
     * @return The sensor type (SOIL_MOISTURE)
     */
    SensorType GetType() const override;
};