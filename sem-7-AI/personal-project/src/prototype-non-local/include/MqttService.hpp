#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

class MqttService
{
public:
    MqttService(const char *host,
                uint16_t port,
                const char *username,
                const char *password,
                const char *topicPrefix,
                const char *clientPrefix);

    bool IsConfigured() const;
    bool Connect();
    bool Publish(const String &topicSuffix, const String &payload);
    bool IsConnected() const;

private:
    String BuildTopic(const String &topicSuffix) const;

    const char *host_;
    uint16_t port_;
    const char *username_;
    const char *password_;
    const char *topicPrefix_;
    const char *clientPrefix_;
    WiFiClientSecure secureClient_;
    PubSubClient mqttClient_;
};
