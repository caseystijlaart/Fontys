#include <Arduino.h>
#include "MonitoringController.hpp"
#include <ctime>

MonitoringController::MonitoringController(
    SensorManager &sensorMgr,
    Prediction &pred,
    ICommunication &comm)
    : sensorManager(sensorMgr), prediction(pred), communication(comm)
{
}

void MonitoringController::UpdateConfiguration()
{
    configuration = communication.ReceiveConfig();
}

void MonitoringController::RunCycle()
{
    SensorData data = sensorManager.ReadAll();
    PredictionResult result = prediction.Predict(data, configuration);

    // communication.Send(data);
    // Serial.println(result.probabilities[2], 4);

    Serial.print("1,");
    Serial.print(data.humidity, 4);
    Serial.print(",");
    Serial.print(data.temperature, 4);
    Serial.print(",");
    Serial.print(data.soilMoisture, 4);
    Serial.print(",");
    Serial.print(result.health == PlantHealth::HEALTHY ? "HEALTHY" : (result.health == PlantHealth::HIGH_STRESS ? "HIGH_STRESS" : "MODERATE_STRESS"));
    Serial.print(",");
    Serial.println(result.confidence, 4);
}