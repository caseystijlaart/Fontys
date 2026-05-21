#pragma once

#include <Arduino.h>
#include <vector>

#include "PlantTypes.hpp"

class ThresholdWarningService
{
public:
    struct Warning
    {
        String metric;
        String action;
        float latestValue;
        float threshold;
    };

    std::vector<Warning> BuildWarningsFromLog(const String &logFilePath,
                                              const pof02::PlantRuleProfile &profile,
                                              std::int64_t durationSeconds = 5LL * 24LL * 60LL * 60LL) const;
};
