#include <Arduino.h>
#include "Communication.hpp"

bool Communication::Init()
{
    return true;
}

bool Communication::Send(const SensorData &data)
{
    return true;
}

PlantConfiguration Communication::ReceiveConfig()
{
    PlantConfiguration config; // Example configuration To adjust
    return config;
}