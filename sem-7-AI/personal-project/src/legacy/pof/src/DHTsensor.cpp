#include "DHTSensor.hpp"

float DHTSensor::cachedHumidity = NAN;
float DHTSensor::cachedTemperature = NAN;
unsigned long DHTSensor::lastReadTime = 0;
bool DHTSensor::initialized = false;

DHTSensor::DHTSensor(DHT& dhtRef, SensorType sensorType)
    : dht(dhtRef), type(sensorType) {}

bool DHTSensor::Init() {
    if (!initialized) {
        dht.begin();
        initialized = true;
    }
    return true;
}

void DHTSensor::updateCache() {
    unsigned long now = millis();

    if (now - lastReadTime < READ_INTERVAL_MS) {
        return;
    }

    lastReadTime = now;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (!isnan(humidity)) {
        cachedHumidity = humidity;
    }

    if (!isnan(temperature)) {
        cachedTemperature = temperature;
    }
}

float DHTSensor::GetData() {
    updateCache();

    switch (type) {
        case SensorType::HUMIDITY:
            return cachedHumidity;
        case SensorType::TEMPERATURE:
            return cachedTemperature;
        default:
            return NAN;
    }
}

SensorType DHTSensor::GetType() const {
    return type;
}