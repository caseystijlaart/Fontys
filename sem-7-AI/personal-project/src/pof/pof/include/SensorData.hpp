#pragma once

/** @brief Struct to hold sensor data
 * This struct encapsulates the various sensor readings that are used for predicting plant health.
 * It includes humidity, temperature, soil moisture, and light intensity values.
 */
struct SensorData
{
    float humidity = 0.0f;
    float temperature = 0.0f;
    float soilMoisture = 0.0f;
    float light = 0.0f;
};