#include "ThresholdWarningService.hpp"

#include <FS.h>
#include <LittleFS.h>

std::vector<ThresholdWarningService::Warning> ThresholdWarningService::BuildWarningsFromLog(const String &logFilePath,
                                                                                             const pof02::PlantRuleProfile &profile,
                                                                                             const std::int64_t durationSeconds) const
{
    std::vector<Warning> warnings;
    if (logFilePath.length() == 0)
    {
        return warnings;
    }

    File file = LittleFS.open(logFilePath, FILE_READ);
    if (!file)
    {
        return warnings;
    }

    file.readStringUntil('\n');

    std::int64_t newestUnix = 0;
    float latestSoil = 0.0f, latestTemp = 0.0f, latestHumidity = 0.0f, latestLight = 0.0f;

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
        {
            continue;
        }

        float values[19];
        int valueIndex = 0;
        int start = 0;
        while (valueIndex < 19 && start <= line.length())
        {
            int comma = line.indexOf(',', start);
            String token = comma >= 0 ? line.substring(start, comma) : line.substring(start);
            token.trim();
            if (token.startsWith("\"") && token.endsWith("\"") && token.length() >= 2)
            {
                token = token.substring(1, token.length() - 1);
            }
            values[valueIndex++] = token.toFloat();
            if (comma < 0)
            {
                break;
            }
            start = comma + 1;
        }
        if (valueIndex < 9)
        {
            continue;
        }

        const std::int64_t unixTime = static_cast<std::int64_t>(values[4]);
        if (unixTime > newestUnix)
        {
            newestUnix = unixTime;
            latestSoil = values[5];
            latestTemp = values[6];
            latestHumidity = values[7];
            latestLight = values[8];
        }
    }
    file.close();

    if (newestUnix <= 0)
    {
        return warnings;
    }

    const std::int64_t windowStart = newestUnix - durationSeconds;

    file = LittleFS.open(logFilePath, FILE_READ);
    if (!file)
    {
        return warnings;
    }
    file.readStringUntil('\n');

    bool soilAlwaysHigh = true, tempAlwaysHigh = true, humidityAlwaysHigh = true, lightAlwaysHigh = true;
    bool sawWindowData = false;
    std::int64_t oldestInWindow = 0;

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
        {
            continue;
        }

        float values[19];
        int valueIndex = 0;
        int start = 0;
        while (valueIndex < 19 && start <= line.length())
        {
            int comma = line.indexOf(',', start);
            String token = comma >= 0 ? line.substring(start, comma) : line.substring(start);
            token.trim();
            if (token.startsWith("\"") && token.endsWith("\"") && token.length() >= 2)
            {
                token = token.substring(1, token.length() - 1);
            }
            values[valueIndex++] = token.toFloat();
            if (comma < 0)
            {
                break;
            }
            start = comma + 1;
        }
        if (valueIndex < 9)
        {
            continue;
        }

        const std::int64_t unixTime = static_cast<std::int64_t>(values[4]);
        if (unixTime < windowStart)
        {
            continue;
        }

        sawWindowData = true;
        if (oldestInWindow == 0 || unixTime < oldestInWindow)
        {
            oldestInWindow = unixTime;
        }

        soilAlwaysHigh = soilAlwaysHigh && (values[5] > profile.soilMoistureThresholds.highMin);
        tempAlwaysHigh = tempAlwaysHigh && (values[6] > profile.temperatureThresholds.highMin);
        humidityAlwaysHigh = humidityAlwaysHigh && (values[7] > profile.humidityThresholds.highMin);
        lightAlwaysHigh = lightAlwaysHigh && (values[8] > profile.lightThresholds.highMin);
    }
    file.close();

    if (!(sawWindowData && oldestInWindow <= windowStart))
    {
        return warnings;
    }

    if (soilAlwaysHigh)
        warnings.push_back({"Soil moisture", "Pause watering and improve drainage.", latestSoil, profile.soilMoistureThresholds.highMin});
    if (tempAlwaysHigh)
        warnings.push_back({"Temperature", "Move the plant to a cooler place.", latestTemp, profile.temperatureThresholds.highMin});
    if (humidityAlwaysHigh)
        warnings.push_back({"Humidity", "Increase airflow and reduce nearby moisture sources.", latestHumidity, profile.humidityThresholds.highMin});
    if (lightAlwaysHigh)
        warnings.push_back({"Light", "Place the plant in a less direct sunlight spot.", latestLight, profile.lightThresholds.highMin});

    return warnings;
}
