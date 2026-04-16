#include "SensorManager.hpp"

SensorManager::SensorManager() : sensorCount(0) {
    for (int i = 0; i < MAX_SENSORS; i++) {
        sensors[i] = nullptr;
    }
}

bool SensorManager::Init(ISensor* sensorList[], int count) {
    if (count > MAX_SENSORS) {
        return false;
    }

    sensorCount = count;

    for (int i = 0; i < count; i++) {
        sensors[i] = sensorList[i];
        if (sensors[i] == nullptr || !sensors[i]->Init()) {
            return false;
        }
    }

    return true;
}

SensorData SensorManager::ReadAll() {
    SensorData data;

    for (int i = 0; i < sensorCount; i++) {
        if (sensors[i] == nullptr) continue;

        switch (sensors[i]->GetType()) {
            case SensorType::HUMIDITY:
                data.humidity = sensors[i]->GetData();
                break;

            case SensorType::TEMPERATURE:
                data.temperature = sensors[i]->GetData();
                break;

            case SensorType::SOIL_MOISTURE:
                data.soilMoisture = sensors[i]->GetData();
                break;

            case SensorType::LIGHT:
                data.light = sensors[i]->GetData();
                break;
        }
    }

    return data;
}