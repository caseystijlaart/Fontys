#include "MqttService.hpp"

MqttService::MqttService(const char *host,
                         uint16_t port,
                         const char *username,
                         const char *password,
                         const char *topicPrefix,
                         const char *clientPrefix)
    : host_(host),
      port_(port),
      username_(username),
      password_(password),
      topicPrefix_(topicPrefix),
      clientPrefix_(clientPrefix),
      mqttClient_(secureClient_)
{
    secureClient_.setInsecure();
    mqttClient_.setServer(host_, port_);
    mqttClient_.setBufferSize(1024);
}

bool MqttService::IsConfigured() const
{
    return String(host_).length() > 0 && port_ > 0 && String(topicPrefix_).length() > 0;
}

bool MqttService::Connect()
{
    if (!IsConfigured())
    {
        Serial.println("MQTT not configured; skipping");
        return false;
    }

    if (mqttClient_.connected())
    {
        return true;
    }

    const String clientId = String(clientPrefix_) + "-" + String(static_cast<unsigned long>(ESP.getEfuseMac()), HEX);

    Serial.print("Connecting to MQTT broker: ");
    Serial.print(host_);
    Serial.print(":");
    Serial.println(port_);

    const bool connected = mqttClient_.connect(clientId.c_str(), username_, password_);
    if (!connected)
    {
        Serial.print("MQTT connection failed, state=");
        Serial.println(mqttClient_.state());
        return false;
    }

    Serial.println("MQTT connected");
    return true;
}

bool MqttService::Publish(const String &topicSuffix, const String &payload)
{
    if (!Connect())
    {
        return false;
    }

    mqttClient_.loop();
    const String topic = BuildTopic(topicSuffix);
    const bool published = mqttClient_.publish(topic.c_str(), payload.c_str(), true);

    if (!published)
    {
        Serial.print("MQTT publish failed for topic ");
        Serial.println(topic);
        return false;
    }

    Serial.print("MQTT publish ok: ");
    Serial.println(topic);
    return true;
}

bool MqttService::IsConnected() 
{
    return mqttClient_.connected();
}

String MqttService::BuildTopic(const String &topicSuffix) const
{
    String topic = topicPrefix_;
    if (!topic.endsWith("/"))
    {
        topic += "/";
    }
    topic += topicSuffix;
    return topic;
}
