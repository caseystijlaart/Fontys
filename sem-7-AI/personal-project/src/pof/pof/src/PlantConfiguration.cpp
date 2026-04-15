#include "PlantConfiguration.hpp"

PlantConfiguration::PlantConfiguration()
{
    config.moistureThreshold = 60.0f;
    config.temperatureThreshold = 25.0f;
}

PlantConfiguration::PlantConfiguration(float moistureThreshold, float temperatureThreshold)
{
    config.moistureThreshold = moistureThreshold;
    config.temperatureThreshold = temperatureThreshold;
}

const PlantConfig &PlantConfiguration::getConfig() const
{
    return config;
}

bool PlantConfiguration::SetMoistureThreshold(float threshold)
{
    if (threshold < 0.0f || threshold > 100.0f)
    {
        return false;
    }
    config.moistureThreshold = threshold;
    return true;
}

bool PlantConfiguration::SetTemperatureThreshold(float threshold)
{
    if (threshold < 0.0f || threshold > 100.0f)
    {
        return false;
    }
    config.temperatureThreshold = threshold;
    return true;
}

float PlantConfiguration::GetHumidityThreshold() const
{
    return config.humidityThreshold;
}

float PlantConfiguration::GetMoistureThreshold() const
{
    return config.moistureThreshold;
}
