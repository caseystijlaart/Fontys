#include "TempHumiditySensor.hpp"

#include <Arduino.h>
#include <DHT.h>

namespace pof02 {

TempHumiditySensor::TempHumiditySensor(int pin, int dhtType) : pin_(pin), dhtType_(dhtType) {}

bool TempHumiditySensor::Init() {
    (void)pin_;
    (void)dhtType_;
    return true;
}

std::pair<float, float> TempHumiditySensor::Read() const {
    static DHT dht(pin_, dhtType_);
    static bool dhtInitialized = false;
    if (!dhtInitialized) {
        dht.begin();
        dhtInitialized = true;
    }

    const float temperature = dht.readTemperature();
    const float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        return {23.0f, 60.0f};
    }
    return {temperature, humidity};
}

} // namespace pof02
