#pragma once

#include <Arduino.h>

class TimeService;

class WiFiCommunication
{
public:
    WiFiCommunication(const char *ssid, const char *password, TimeService &timeService);

    void Connect();
    bool IsConnected() const;
    bool HasCredentials() const;
    IPAddress LocalIp() const;

private:
    const char *ssid_;
    const char *password_;
    TimeService &timeService_;
};
