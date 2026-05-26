#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WebServer.h>

#include "FileStorageService.hpp"
#include "MonitoringSystem.hpp"
#include "MqttService.hpp"
#include "TimeService.hpp"
#include "WiFiCommunication.hpp"

using namespace pof02;

namespace
{
const unsigned long INTERVAL = 7200000UL;
const char *PLANT_SETTINGS_FILE = "/plant_settings.txt";
const char *LAST_STATE_FILE = "/last_state.txt";

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

#ifndef PLANT_LABEL
#define PLANT_LABEL "UNSET_PLANT"
#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME "UNSET_DEVICE"
#endif

#ifndef DEVICE_ID
#define DEVICE_ID 0
#endif

#ifndef ENABLE_WIFI_DOWNLOAD
#define ENABLE_WIFI_DOWNLOAD 1
#endif

#ifndef MQTT_HOST
#define MQTT_HOST ""
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 8883
#endif

#ifndef MQTT_USERNAME
#define MQTT_USERNAME ""
#endif

#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD ""
#endif

#ifndef MQTT_TOPIC_PREFIX
#define MQTT_TOPIC_PREFIX "fontys/plants"
#endif

#ifndef MQTT_CLIENT_PREFIX
#define MQTT_CLIENT_PREFIX "algaonema"
#endif

// Calibrated for a chunkier mix (~50% soil / 25% perlite / 25% bark)
// that retains moisture differently than dense potting soil.
SoilMoistureSensor soil(34, 3500.0f, 1450.0f);
TempHumiditySensor dht(4, 22);
LightSensor light(35);
MLLayer ml(MLBackend::TINYML_TFLM);
RecommendationEngine recEngine;
MonitoringSystem monitoringSystem(soil, dht, light, ml, recEngine, PlantRuleProfile{}, 24);

TimeService timeService;
WiFiCommunication wifiCommunication(WIFI_SSID, WIFI_PASSWORD, timeService);
MqttService mqttService(MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, MQTT_TOPIC_PREFIX, MQTT_CLIENT_PREFIX);
FileStorageService fileStorageService(timeService);
WebServer server(80);
MonitoringCycleResult latestResult{};
bool hasLatestResult = false;

unsigned long lastRun = 0;

String EscapeJsonString(const String &input)
{
    String output = input;
    output.replace("\\", "\\\\");
    output.replace("\"", "\\\"");
    output.replace("\n", "\\n");
    output.replace("\r", "\\r");
    return output;
}

void RunMonitoringCycle()
{
    latestResult = monitoringSystem.RunCycleDetailed();
    hasLatestResult = true;

    String payload = "{";
    payload += "\"timestamp_utc\":\"" + EscapeJsonString(timeService.FormatTimestampLocal(latestResult.snapshot.unixTime)) + "\",";
    payload += "\"plant_label\":\"" + EscapeJsonString(String(PLANT_LABEL)) + "\",";
    payload += "\"device_id\":" + String(DEVICE_ID) + ",";
    payload += "\"soil_moisture_pct\":" + String(latestResult.snapshot.soilMoisturePct, 2) + ",";
    payload += "\"temperature_c\":" + String(latestResult.snapshot.temperatureC, 2) + ",";
    payload += "\"humidity_pct\":" + String(latestResult.snapshot.humidityPct, 2) + ",";
    payload += "\"light_level_pct\":" + String(latestResult.snapshot.lightLevelPct, 2) + ",";
    payload += "\"action_water\":" + String(latestResult.recommendation.water ? "true" : "false") + ",";
    payload += "\"action_reduce_temp\":" + String(latestResult.recommendation.reduceTemp ? "true" : "false") + ",";
    payload += "\"action_increase_light\":" + String(latestResult.recommendation.increaseLight ? "true" : "false") + ",";
    payload += "\"recommendation_summary\":\"" + EscapeJsonString(String(latestResult.recommendation.summary.c_str())) + "\"";
    payload += "}";

    const bool mqttOk = mqttService.Publish(String(PLANT_LABEL) + "/" + String(DEVICE_ID), payload);

    fileStorageService.SaveLastStateToFile(latestResult, LAST_STATE_FILE);
}

} // namespace

void setup()
{
    Serial.begin(115200);
    delay(200);

    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS mount failed");
        while (true)
        {
        }
    }

    Serial.println("LittleFS mounted");

    fileStorageService.SetImportantFiles(PLANT_SETTINGS_FILE, LAST_STATE_FILE);

    if (wifiCommunication.HasCredentials())
    {
        wifiCommunication.Connect();
    }

    PlantRuleProfile initialProfile;
    initialProfile.plantName = PLANT_LABEL;
    initialProfile.deviceName = DEVICE_NAME;
    initialProfile.deviceId = String(DEVICE_ID).c_str();
    monitoringSystem.SetPlantProfile(initialProfile);
    fileStorageService.SavePlantSettingsToFile(initialProfile, PLANT_SETTINGS_FILE);

    if (!monitoringSystem.Init())
    {
        Serial.println("Failed to initialize monitoring system");
        while (true)
        {
        }
    }

    Serial.println("Monitoring system initialized");
    RunMonitoringCycle();

    lastRun = millis();
}

void loop()
{
#if ENABLE_WIFI_DOWNLOAD
    server.handleClient();
#endif

    const unsigned long now = millis();
    if (now - lastRun < INTERVAL)
    {
        return;
    }

    lastRun = now;

    if (wifiCommunication.HasCredentials() && !wifiCommunication.IsConnected())
    {
        wifiCommunication.Connect();
    }
    else if (!timeService.IsSynced() && wifiCommunication.IsConnected())
    {
        timeService.SyncTimeWithNtp();
    }

    RunMonitoringCycle();
}