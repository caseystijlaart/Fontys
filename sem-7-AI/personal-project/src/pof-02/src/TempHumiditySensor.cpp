#include "TempHumiditySensor.hpp"

#ifdef ARDUINO
#include <Arduino.h>
#include <DHT.h>
#else
#include <cmath>
#include <ctime>
#endif

namespace pof02 {

TempHumiditySensor::TempHumiditySensor(int pin, int dhtType) : pin_(pin), dhtType_(dhtType) {}

bool TempHumiditySensor::Init() {
#ifdef ARDUINO
    (void)pin_;
    (void)dhtType_;
#endif
    return true;
}

std::pair<float, float> TempHumiditySensor::Read() const {
#ifdef ARDUINO
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
#else
    const double t = static_cast<double>(std::time(nullptr));
    const float temperature = static_cast<float>(23.0 + 4.5 * std::sin(t / 1200.0));
    const float humidity = static_cast<float>(63.0 + 12.0 * std::cos(t / 1000.0));
    return {temperature, humidity};
#endif
}

} // namespace pof02
