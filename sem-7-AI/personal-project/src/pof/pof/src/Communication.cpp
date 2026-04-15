#include <Arduino.h>
#include "Communication.hpp"

bool Communication::Init() {
    return true;
}

bool Communication::Send(const SensorData& data) {
    Serial.println("Sending sensor data:");
    Serial.print("Humidity: ");
    Serial.println(data.humidity);
    Serial.print("Temperature: ");
    Serial.println(data.temperature);
    Serial.print("Soil Moisture: ");
    Serial.println(data.soilMoisture);
    return true;
}

PlantConfiguration Communication::ReceiveConfig() {
    PlantConfiguration config(30.0f, 25.0f, 80.0f, 245.0f); // Example configuration To adjust
    return config;
}