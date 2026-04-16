#include <Arduino.h>
#include <DHT.h>
#include <ctime>

#include "DHTSensor.hpp"
#include "SoilMoistureSensor.hpp"
#include "SensorManager.hpp"
#include "Prediction.hpp"
#include "Communication.hpp"
#include "MonitoringController.hpp"

#define DHT_PIN 4
#define DHT_TYPE DHT22
#define SOIL_PIN 34

#ifndef DEVICE_ID
#define DEVICE_ID 0
#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME "Unknown Device"
#endif

DHT dht(DHT_PIN, DHT_TYPE);

DHTSensor humiditySensor(dht, SensorType::HUMIDITY);
DHTSensor temperatureSensor(dht, SensorType::TEMPERATURE);
SoilMoistureSensor soilSensor(SOIL_PIN, 3000, 1200);

SensorManager sensorManager;
Prediction prediction;
Communication communication;
MonitoringController *controller = nullptr;

unsigned long lastSampleTime = 0;
const unsigned long sampleIntervalMs = 150000; // 2.5 minutes

void setup()
{
    Serial.begin(115200);
    delay(2000);

    analogReadResolution(12);

    ISensor *sensors[] = {
        &humiditySensor,
        &temperatureSensor,
        &soilSensor};

    bool sensorsOk = sensorManager.Init(sensors, 3);
    bool predictionOk = prediction.Init();
    bool communicationOk = communication.Init();

    if (!sensorsOk || !predictionOk || !communicationOk)
    {
        Serial.println("System initialization failed.");
        while (true)
        {
            delay(1000);
        }
    }

    controller = new MonitoringController(sensorManager, prediction, communication);
    controller->UpdateConfiguration();

    Serial.println("System initialized.");
    controller->RunCycle();
}

void loop()
{
    unsigned long now = millis();

    if (controller != nullptr && (now - lastSampleTime >= sampleIntervalMs))
    {
        controller->RunCycle();
        lastSampleTime = now;
    }
}