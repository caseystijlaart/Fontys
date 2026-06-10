#pragma once

#include "ICommunication.hpp"

class Communication : public ICommunication
{
public:
    /** @brief Initialize the communication interface
     * @return true if initialization was successful, false otherwise
     */
    bool Init() override;

    /** @brief Send sensor data to the server
     * @param data The sensor data to be sent
     * @return true if the data was sent successfully, false otherwise
     */
    bool Send(const SensorData &data) override;
    
    /** @brief Receive plant configuration from the server
     * @return The received plant configuration
     */
    PlantConfiguration ReceiveConfig() override;
};