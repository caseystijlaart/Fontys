#pragma once

#include "SensorType.hpp"

class ISensor
{
public:
    /** @brief Initialize the sensor
     * @return true if initialization was successful, false otherwise
     */
    virtual bool Init() = 0;

    /** @brief Get the type of sensor
     * @return The sensor type (HUMIDITY or TEMPERATURE)
     */

    virtual SensorType GetType() const = 0;

    /** @brief Get the sensor data
     * @return The sensor data value
     */
    virtual float GetData() = 0;

    /** @brief Destructor for ISensor
     */
    virtual ~ISensor() = default;
};