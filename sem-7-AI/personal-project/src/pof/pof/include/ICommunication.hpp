#pragma once

#include <ctime>

#include "SensorData.hpp"
#include "PlantConfiguration.hpp"

class ICommunication
{
public:
    /** @brief Initialize the communication interface
     * @return true if initialization was successful, false otherwise
     */
    virtual bool Init() = 0;

    /** @brief Send sensor data to the server
     * @param data The sensor data to be sent
     * @return true if the data was sent successfully, false otherwise
     */
    virtual bool Send(const SensorData &data) = 0;

    /** @brief Receive plant configuration from the server
     * @return The received plant configuration
     */
    virtual PlantConfiguration ReceiveConfig() = 0;

    /** @brief Destructor for ICommunication
     */
    virtual ~ICommunication() = default;
};