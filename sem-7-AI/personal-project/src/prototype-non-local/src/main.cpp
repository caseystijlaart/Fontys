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
#define MQTT_CLIENT_PREFIX "esp32-plant"
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
unsigned long logCount = 0;

PreferenceBand ParsePreferenceBand(const String &input)
{
    if (input.equalsIgnoreCase("pLow") || input.equalsIgnoreCase("low"))
    {
        return PreferenceBand::pLow;
    }
    if (input.equalsIgnoreCase("pHigh") || input.equalsIgnoreCase("high"))
    {
        return PreferenceBand::pHigh;
    }
    return PreferenceBand::pMid;
}

String PreferenceToString(PreferenceBand band)
{
    switch (band)
    {
    case PreferenceBand::pLow:
        return "pLow";
    case PreferenceBand::pHigh:
        return "pHigh";
    case PreferenceBand::pMid:
    default:
        return "pMid";
    }
}

String EscapeJsonString(const String &input)
{
    String output = input;
    output.replace("\\", "\\\\");
    output.replace("\"", "\\\"");
    output.replace("\n", "\\n");
    output.replace("\r", "\\r");
    return output;
}

String GetCurrentLogDirectory()
{
    String currentLogDir = "/logs";

    if (server.hasArg("dir"))
    {
        currentLogDir = server.arg("dir");
    }

    currentLogDir = fileStorageService.NormalizeFilePath(currentLogDir);

    if (!currentLogDir.startsWith("/logs"))
    {
        currentLogDir = "/logs";
    }

    if (!fileStorageService.IsSafePath(currentLogDir))
    {
        currentLogDir = "/logs";
    }

    return currentLogDir;
}

PlantRuleProfile BuildPlantSettingsFromForm(const PlantRuleProfile &existingProfile)
{
    PlantRuleProfile profile = existingProfile;

    if (server.hasArg("plantName"))
    {
        profile.plantName = server.arg("plantName").c_str();
    }

    profile.preferences.soilMoisture = ParsePreferenceBand(server.arg("soilPreference"));
    profile.preferences.temperature = ParsePreferenceBand(server.arg("temperaturePreference"));
    profile.preferences.humidity = ParsePreferenceBand(server.arg("humidityPreference"));
    profile.preferences.light = ParsePreferenceBand(server.arg("lightPreference"));

    return profile;
}

void HandleDelete()
{
    if (!server.hasArg("file"))
    {
        server.send(400, "text/plain", "Missing 'file' query parameter");
        return;
    }

    const String rawPath = server.arg("file");
    if (!fileStorageService.IsSafePath(rawPath))
    {
        server.send(400, "text/plain", "Invalid file path");
        return;
    }

    const String filePath = fileStorageService.NormalizeFilePath(rawPath);

    if (!LittleFS.exists(filePath))
    {
        server.send(404, "text/plain", "File not found");
        return;
    }

    if (!LittleFS.remove(filePath))
    {
        server.send(500, "text/plain", "Failed to delete file");
        return;
    }

    Serial.print("Deleted file: ");
    Serial.println(filePath);

    String redirectTarget = "/";
    const int lastSlash = filePath.lastIndexOf('/');
    if (lastSlash > 0)
    {
        redirectTarget = "/?dir=" + filePath.substring(0, lastSlash);
    }

    server.sendHeader("Location", redirectTarget);
    server.send(303);
}

void HandleDeleteFolder()
{
    if (!server.hasArg("folder"))
    {
        server.send(400, "text/plain", "Missing 'folder' form field");
        return;
    }

    const String rawPath = server.arg("folder");

    if (!fileStorageService.IsSafePath(rawPath))
    {
        server.send(400, "text/plain", "Invalid folder path");
        return;
    }

    const String folderPath = fileStorageService.NormalizeFilePath(rawPath);

    if (!folderPath.startsWith("/logs") || folderPath == "/logs")
    {
        server.send(400, "text/plain", "Refusing to delete protected folder");
        return;
    }

    const int lastSlash = folderPath.lastIndexOf('/');
    String parentDir = "/logs";

    if (lastSlash > 0)
    {
        parentDir = folderPath.substring(0, lastSlash);
    }

    if (parentDir.length() == 0 || !parentDir.startsWith("/logs"))
    {
        parentDir = "/logs";
    }

    if (!fileStorageService.DeleteFolderRecursive(folderPath))
    {
        server.send(500, "text/plain", "Failed to delete folder");
        return;
    }

    server.sendHeader("Location", "/?dir=" + parentDir);
    server.send(303);
}

void HandleRoot()
{
    const PlantRuleProfile profile = monitoringSystem.GetPlantProfile();
    const String currentLogDir = GetCurrentLogDirectory();

    String html;
    html.reserve(24000);

    html += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>POC-03 Plant Settings</title>";
    html += "</head><body style='font-family:Arial,sans-serif;max-width:1100px;margin:0 auto;padding:20px;line-height:1.4;'>";
    html += "<h1>POC-03 Plant Settings & Device Files</h1>";
    html += "<p><strong>Device:</strong> ";
    html += DEVICE_NAME;
    html += " (ID " + String(DEVICE_ID) + ")</p>";
    html += "<p><strong>Plant:</strong> " + String(profile.plantName.c_str()) + "</p>";
    html += "<p><strong>Time sync:</strong> ";
    html += timeService.IsSynced() ? "NTP synced" : "Not synced";
    html += "</p>";

    const std::int64_t nowUnix = timeService.GetCurrentUnixTimeUtc();
    if (nowUnix > 0)
    {
        html += "<p><strong>Current local time:</strong> " + timeService.FormatTimestampLocal(nowUnix) + "</p>";
    }

    html += "<h2>Current Plant Status</h2>";
    html += "<div id='plantStatus' style='padding:10px;border-radius:8px;font-weight:bold;background:#f2f2f2;color:#333;'>Loading...</div>";
    html += "<table style='margin-top:10px;border-collapse:collapse;width:100%;max-width:700px;'>";
    html += "<tr><th style='text-align:left;border-bottom:1px solid #ddd;padding:8px;'>Metric</th><th style='text-align:left;border-bottom:1px solid #ddd;padding:8px;'>Value</th></tr>";
    html += "<tr><td style='padding:8px;border-bottom:1px solid #eee;'>Expected watering</td><td id='expectedWatering' style='padding:8px;border-bottom:1px solid #eee;'>-</td></tr>";
    html += "<tr><td style='padding:8px;'>Suggestion</td><td id='suggestion' style='padding:8px;'>-</td></tr>";
    html += "</table><hr>";

    html += "<h2>Adjust current plant settings</h2>";
    html += "<p>This ESP32 is already linked to a specific device. Use this page only to adjust the active plant preferences.</p>";
    html += "<form id='settingsForm'>";
    html += "<label>Plant name<br><input name='plantName' required style='width:320px;padding:6px;' value='" + String(profile.plantName.c_str()) + "' /></label><br><br>";

    html += "<label>Humidity preference<br><select name='humidityPreference' style='padding:6px;'>";
    html += "<option value='pLow'>pLow</option><option value='pMid'>pMid</option><option value='pHigh'>pHigh</option>";
    html += "</select></label><br><br>";

    html += "<label>Temperature preference<br><select name='temperaturePreference' style='padding:6px;'>";
    html += "<option value='pLow'>pLow</option><option value='pMid'>pMid</option><option value='pHigh'>pHigh</option>";
    html += "</select></label><br><br>";

    html += "<label>Soil moisture preference<br><select name='soilPreference' style='padding:6px;'>";
    html += "<option value='pLow'>pLow</option><option value='pMid'>pMid</option><option value='pHigh'>pHigh</option>";
    html += "</select></label><br><br>";

    html += "<label>Light preference<br><select name='lightPreference' style='padding:6px;'>";
    html += "<option value='pLow'>pLow</option><option value='pMid'>pMid</option><option value='pHigh'>pHigh</option>";
    html += "</select></label><br><br>";

    html += "<button type='submit' style='padding:8px 14px;'>Save settings</button>";
    html += " <button type='button' onclick='location.reload()' style='padding:8px 14px;'>Refresh page</button></form>";
    html += "<pre id='result' style='background:#f5f5f5;padding:10px;white-space:pre-wrap;'></pre><hr>";

    html += "<h2>Important files</h2><p>Total important files found: ";
    html += String(fileStorageService.CountImportantFiles(PLANT_SETTINGS_FILE, LAST_STATE_FILE));
    html += "</p>";
    html += fileStorageService.BuildFilteredFilesHtmlTable(&FileStorageService::IsImportantFile,
                                                           PLANT_SETTINGS_FILE,
                                                           LAST_STATE_FILE);

    html += "<hr><h2>Log files</h2>";
    html += "<p>Total log files found: " + String(fileStorageService.CountLogFiles()) + "</p>";
    html += fileStorageService.BuildFolderBrowserHtmlTable(currentLogDir,
                                                           &FileStorageService::IsLogFile,
                                                           PLANT_SETTINGS_FILE,
                                                           LAST_STATE_FILE);
    html += "<p style='margin-top:16px;'><a href='/latest'>Download latest log</a></p>";
    html += "<p><a href='/list'>View file list as plain text</a></p>";

    html += "<script>";
    html += "function formatDuration(minutes){if(minutes===undefined||minutes===null||isNaN(minutes))return '-';if(minutes<=0)return 'now';if(minutes<60)return Math.round(minutes)+' minutes';const h=minutes/60;if(h<48)return (h<10?h.toFixed(1):Math.round(h))+' hours';const d=h/24;return (d<10?d.toFixed(1):Math.round(d))+' days';}";
    html += "function riskInfo(risk){if(risk===0)return {label:'Healthy',bg:'#d1fae5',fg:'#065f46'};if(risk===2)return {label:'High stress',bg:'#fee2e2',fg:'#991b1b'};return {label:'Moderate stress',bg:'#fef3c7',fg:'#92400e'};}";
    html += "async function loadSettings(){const r=await fetch('/api/plant-settings');const s=await r.json();const form=document.getElementById('settingsForm');form.plantName.value=s.plantName;form.humidityPreference.value=s.humidityPreference;form.temperaturePreference.value=s.temperaturePreference;form.soilPreference.value=s.soilPreference;form.lightPreference.value=s.lightPreference;}";
    html += "async function loadPlantStatus(){try{const r=await fetch('/api/status');if(!r.ok)return;const s=await r.json();const info=riskInfo(s.riskClass);const card=document.getElementById('plantStatus');card.textContent=info.label;card.style.background=info.bg;card.style.color=info.fg;document.getElementById('expectedWatering').textContent='in '+formatDuration(s.predictedMinutesToWater);document.getElementById('suggestion').textContent=s.suggestion;}catch(e){console.log(e);}}";
    html += "document.getElementById('settingsForm').addEventListener('submit', async (e)=>{e.preventDefault();const fd=new FormData(e.target);const body=new URLSearchParams(fd);const response=await fetch('/api/plant-settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});document.getElementById('result').textContent=await response.text();await loadSettings();});";
    html += "loadSettings();loadPlantStatus();setInterval(loadPlantStatus,15000);";
    html += "</script></body></html>";

    server.send(200, "text/html", html);
}

void HandleStatus()
{
    if (!hasLatestResult)
    {
        server.send(503, "application/json", "{\"error\":\"No monitoring data yet\"}");
        return;
    }

    const auto &rec = latestResult.recommendation;
    const auto &mlResult = latestResult.mlResult;

    String suggestion = rec.water ? "Water soon and re-check soil moisture." : "No immediate watering required; continue monitoring.";

    if (rec.increaseLight)
    {
        suggestion += " Increase light exposure.";
    }

    if (rec.reduceTemp)
    {
        suggestion += " Reduce temperature slightly.";
    }

    String json = "{";
    json += "\"riskClass\":" + String(static_cast<int>(mlResult.risk)) + ",";
    json += "\"predictedMinutesToWater\":" + String(rec.predictedMinutesToWater, 1) + ",";
    json += "\"suggestion\":\"" + EscapeJsonString(suggestion) + "\"";
    json += "}";

    server.send(200, "application/json", json);
}

void HandleDownload()
{
    if (!server.hasArg("file"))
    {
        server.send(400, "text/plain", "Missing 'file' query parameter");
        return;
    }

    const String rawPath = server.arg("file");
    if (!fileStorageService.IsSafePath(rawPath))
    {
        server.send(400, "text/plain", "Invalid file path");
        return;
    }

    const String filePath = fileStorageService.NormalizeFilePath(rawPath);
    if (!LittleFS.exists(filePath))
    {
        server.send(404, "text/plain", "File not found");
        return;
    }

    File file = LittleFS.open(filePath, FILE_READ);
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=\"" + filePath.substring(1) + "\"");
    server.streamFile(file, fileStorageService.GetContentType(filePath));
    file.close();
}

void HandleView()
{
    if (!server.hasArg("file"))
    {
        server.send(400, "text/plain", "Missing 'file' query parameter");
        return;
    }

    const String rawPath = server.arg("file");
    if (!fileStorageService.IsSafePath(rawPath))
    {
        server.send(400, "text/plain", "Invalid file path");
        return;
    }

    const String filePath = fileStorageService.NormalizeFilePath(rawPath);
    if (!LittleFS.exists(filePath))
    {
        server.send(404, "text/plain", "File not found");
        return;
    }

    if (!fileStorageService.IsViewableTextFile(filePath))
    {
        server.send(415, "text/plain", "Viewing not supported for this file type");
        return;
    }

    File file = LittleFS.open(filePath, FILE_READ);
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }

    String content;
    content.reserve(file.size() + 64);

    while (file.available())
    {
        content += char(file.read());
    }

    file.close();

    String backTarget = "/";
    const int lastSlash = filePath.lastIndexOf('/');
    if (lastSlash > 0)
    {
        backTarget = "/?dir=" + filePath.substring(0, lastSlash);
    }

    String html;
    html.reserve(content.length() + 1200);
    html += "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>View file</title></head><body style='font-family:Arial,sans-serif;padding:20px;'>";
    html += "<h1>Viewing " + filePath + "</h1>";
    html += "<p><a href='" + backTarget + "'>Back</a> | <a href='/download?file=" + rawPath + "'>Download</a></p>";
    html += "<pre style='white-space:pre-wrap;word-wrap:break-word;background:#f5f5f5;padding:12px;border:1px solid #ddd;'>";
    html += content;
    html += "</pre></body></html>";

    server.send(200, "text/html", html);
}

void HandleLatest()
{
    const String latestFile = fileStorageService.FindLatestLogFile();
    if (latestFile.length() == 0)
    {
        server.send(404, "text/plain", "No log files found");
        return;
    }

    File file = LittleFS.open(latestFile, FILE_READ);
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open latest log file");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=\"" + latestFile.substring(1) + "\"");
    server.streamFile(file, "text/csv");
    file.close();
}

void HandleList()
{
    String text;
    text.reserve(4096);

    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        server.send(500, "text/plain", "Could not open root directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        text += String(file.name()) + " | " + String(file.size()) + " bytes\n";
        file.close();
        file = root.openNextFile();
    }

    root.close();

    if (text.length() == 0)
    {
        text = "No files found\n";
    }

    server.send(200, "text/plain", text);
}

void HandleGetPlantSettings()
{
    const PlantRuleProfile profile = monitoringSystem.GetPlantProfile();

    String json = "{";
    json += "\"plantName\":\"" + EscapeJsonString(String(profile.plantName.c_str())) + "\",";
    json += "\"deviceId\":\"" + EscapeJsonString(String(profile.deviceId.c_str())) + "\",";
    json += "\"deviceName\":\"" + EscapeJsonString(String(profile.deviceName.c_str())) + "\",";
    json += "\"soilPreference\":\"" + PreferenceToString(profile.preferences.soilMoisture) + "\",";
    json += "\"temperaturePreference\":\"" + PreferenceToString(profile.preferences.temperature) + "\",";
    json += "\"humidityPreference\":\"" + PreferenceToString(profile.preferences.humidity) + "\",";
    json += "\"lightPreference\":\"" + PreferenceToString(profile.preferences.light) + "\"";
    json += "}";

    server.send(200, "application/json", json);
}

void HandleUpdatePlantSettings()
{
    if (!server.hasArg("soilPreference") ||
        !server.hasArg("temperaturePreference") ||
        !server.hasArg("humidityPreference") ||
        !server.hasArg("lightPreference"))
    {
        server.send(400, "text/plain", "Missing one or more preference fields");
        return;
    }

    const PlantRuleProfile profile = BuildPlantSettingsFromForm(monitoringSystem.GetPlantProfile());
    monitoringSystem.SetPlantProfile(profile);
    fileStorageService.SavePlantSettingsToFile(profile, PLANT_SETTINGS_FILE);

    String response = "Updated personalized settings for " + String(profile.plantName.c_str());
    response += " on this ESP32.";
    response += " Preferences: soil=" + server.arg("soilPreference");
    response += ", temperature=" + server.arg("temperaturePreference");
    response += ", humidity=" + server.arg("humidityPreference");
    response += ", light=" + server.arg("lightPreference");

    server.send(200, "text/plain", response);
}

void StartWebServer()
{
#if ENABLE_WIFI_DOWNLOAD
    if (!wifiCommunication.IsConnected())
    {
        Serial.println("WiFi not connected; web server not started");
        return;
    }

    server.on("/", HTTP_GET, HandleRoot);
    server.on("/download", HTTP_GET, HandleDownload);
    server.on("/view", HTTP_GET, HandleView);
    server.on("/latest", HTTP_GET, HandleLatest);
    server.on("/list", HTTP_GET, HandleList);
    server.on("/api/plant-settings", HTTP_GET, HandleGetPlantSettings);
    server.on("/api/plant-settings", HTTP_POST, HandleUpdatePlantSettings);
    server.on("/api/status", HTTP_GET, HandleStatus);
    server.on("/delete", HTTP_POST, HandleDelete);
    server.on("/delete-folder", HTTP_POST, HandleDeleteFolder);
    server.on("/favicon.ico", HTTP_GET, []() { server.send(204); });

    server.onNotFound([]() {
        Serial.print("Not found route: ");
        Serial.print(server.uri());
        Serial.print(" | method: ");
        Serial.println(server.method() == HTTP_GET ? "GET" : server.method() == HTTP_POST ? "POST"
                                                                                              : "OTHER");
        server.send(404, "text/plain", "Route not found");
    });

    server.begin();

    Serial.println("Web server started");
    Serial.print("Open in browser: http://");
    Serial.println(wifiCommunication.LocalIp());
#else
    Serial.println("WiFi download disabled");
#endif
}

void RunMonitoringCycle()
{
    latestResult = monitoringSystem.RunCycleDetailed();
    hasLatestResult = true;

    String payload = "{";
    payload += "\"timestamp_utc\":\"" + EscapeJsonString(timeService.FormatTimestampLocal(latestResult.snapshot.unixTime)) + "\",";
    payload += "\"unix_time\":" + String(static_cast<long long>(latestResult.snapshot.unixTime)) + ",";
    payload += "\"plant_label\":\"" + EscapeJsonString(String(PLANT_LABEL)) + "\",";
    payload += "\"device_name\":\"" + EscapeJsonString(String(DEVICE_NAME)) + "\",";
    payload += "\"device_id\":" + String(DEVICE_ID) + ",";
    payload += "\"soil_moisture_pct\":" + String(latestResult.snapshot.soilMoisturePct, 2) + ",";
    payload += "\"temperature_c\":" + String(latestResult.snapshot.temperatureC, 2) + ",";
    payload += "\"humidity_pct\":" + String(latestResult.snapshot.humidityPct, 2) + ",";
    payload += "\"light_level_pct\":" + String(latestResult.snapshot.lightLevelPct, 2) + ",";
    payload += "\"risk_class\":" + String(static_cast<int>(latestResult.mlResult.risk)) + ",";
    payload += "\"confidence\":" + String(latestResult.mlResult.confidence, 4) + ",";
    payload += "\"prob_healthy\":" + String(latestResult.mlResult.probabilities[0], 6) + ",";
    payload += "\"prob_moderate_stress\":" + String(latestResult.mlResult.probabilities[1], 6) + ",";
    payload += "\"prob_high_stress\":" + String(latestResult.mlResult.probabilities[2], 6) + ",";
    payload += "\"action_water\":" + String(latestResult.recommendation.water ? "true" : "false") + ",";
    payload += "\"action_reduce_temp\":" + String(latestResult.recommendation.reduceTemp ? "true" : "false") + ",";
    payload += "\"action_increase_light\":" + String(latestResult.recommendation.increaseLight ? "true" : "false") + ",";
    payload += "\"recommendation_summary\":\"" + EscapeJsonString(String(latestResult.recommendation.summary.c_str())) + "\"";
    payload += "}";

    const bool mqttOk = mqttService.Publish(String(PLANT_LABEL) + "/" + String(DEVICE_ID), payload);
    if (mqttOk)
    {
        ++logCount;
    }

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

    StartWebServer();
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