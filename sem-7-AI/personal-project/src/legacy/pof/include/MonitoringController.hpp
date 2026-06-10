#pragma once

#include "SensorManager.hpp"
#include "Prediction.hpp"
#include "ICommunication.hpp"
#include "PlantConfiguration.hpp"

class MonitoringController
{
private:
    /* References to the main components of the system */
    SensorManager &sensorManager;
    Prediction &prediction;
    ICommunication &communication;
    PlantConfiguration configuration;

public:
    /** @brief Constructor for MonitoringController
     * @param sensorMgr Reference to the sensor manager
     * @param pred Reference to the prediction instance
     * @param comm Reference to the communication interface
     */
    MonitoringController(
        SensorManager &sensorMgr,
        Prediction &pred,
        ICommunication &comm);

    /** @brief Initialize the monitoring controller
     * @return true if initialization was successful, false otherwise
     */
    void RunCycle();

    /** @brief Update the plant configuration by receiving new values from the server
     * This method can be called periodically to ensure the system is using the latest configuration for predictions.
     */
    void UpdateConfiguration();
};