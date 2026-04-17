#pragma once

namespace pof02 {

class LightSensor {
public:
    explicit LightSensor(int pin);
    float ReadLux() const;

private:
    int pin_;
};

} // namespace pof02
