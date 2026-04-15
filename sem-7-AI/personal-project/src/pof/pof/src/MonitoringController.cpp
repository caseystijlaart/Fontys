#include <Arduino.h>
#include "MonitoringController.hpp"
#include "PlantHealth.hpp"

MonitoringController::MonitoringController(
    SensorManager& sensorMgr,
    Prediction& pred,
    ICommunication& comm
)
    : sensorManager(sensorMgr), prediction(pred), communication(comm) {}

void MonitoringController::UpdateConfiguration() {
    configuration = communication.ReceiveConfig();
}

void MonitoringController::RunCycle() {
    SensorData data = sensorManager.ReadAll();
    PlantHealth health = prediction.Predict(data, configuration);

    communication.Send(data);

    Serial.print("Plant health: ");
    switch (health) {
        case PlantHealth::HEALTHY:
            Serial.println("HEALTHY");
            break;
        case PlantHealth::MODERATE_STRESS:
            Serial.println("MODERATE_STRESS");
            break;
        case PlantHealth::HIGH_STRESS:
            Serial.println("HIGH_STRESS");
            break;
    }

    Serial.println("----------------------");
}