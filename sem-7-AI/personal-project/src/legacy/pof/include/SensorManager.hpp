#pragma once

#include "ISensor.hpp"
#include "SensorData.hpp"

class SensorManager
{
private:
    /* Maximum number of sensors that can be managed */
    static const int MAX_SENSORS = 4;
    ISensor *sensors[MAX_SENSORS];
    int sensorCount;

public:
    /** @brief Constructor for SensorManager */
    SensorManager();

    /** @brief Initialize the sensor manager with a list of sensors
     * @param sensorList Array of sensor pointers
     * @param count Number of sensors in the array
     * @return true if initialization was successful, false otherwise
     */
    bool Init(ISensor *sensorList[], int count);

    /** @brief Read data from all sensors and return it as a SensorData struct
     * @return SensorData struct containing the latest readings from all sensors
     */
    SensorData ReadAll();
};