#include <Arduino.h>
#include "MonitoringController.hpp"

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

    communication.Send(data);

    Serial.print("Plant health: ");
    switch (result.health)
    {
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

    Serial.print("Confidence: ");
    Serial.println(result.confidence, 4);

    Serial.print("P(Healthy): ");
    Serial.println(result.probabilities[0], 4);

    Serial.print("P(High Stress): ");
    Serial.println(result.probabilities[1], 4);

    Serial.print("P(Moderate Stress): ");
    Serial.println(result.probabilities[2], 4);

    Serial.println("----------------------");
}