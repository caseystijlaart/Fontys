#pragma once

#include <Arduino.h>
#include <DHT.h>
#include "ISensor.hpp"

class DHTSensor : public ISensor
{
private:
    /* Reference to the DHT sensor instance and sensor type */
    DHT &dht;
    SensorType type;

    /* Static members to cache sensor readings and manage read intervals */
    static float cachedHumidity;
    static float cachedTemperature;
    static unsigned long lastReadTime;
    static bool initialized;

    /** @brief Minimum interval between sensor reads in milliseconds */
    static constexpr unsigned long READ_INTERVAL_MS = 2000;

    /** @brief Update the cached sensor values if the read interval has passed */
    void updateCache();

public:
    /** @brief Constructor for DHTSensor
     * @param dhtRef Reference to the DHT sensor instance
     * @param sensorType Type of sensor (HUMIDITY or TEMPERATURE)
     */
    DHTSensor(DHT &dhtRef, SensorType sensorType);

    /** @brief Initialize the DHT sensor
     * @return true if initialization was successful, false otherwise
     */
    bool Init() override;

    /** @brief Get the sensor data (humidity or temperature)
     * @return The sensor data value
     */
    float GetData() override;

    /** @brief Get the type of sensor
     * @return The sensor type (HUMIDITY or TEMPERATURE)
     */
    SensorType GetType() const override;
};
