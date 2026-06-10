#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include "FileStorageService.hpp"
#include "MonitoringSystem.hpp"
#include "ThresholdWarningService.hpp"
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

SoilMoistureSensor soil(34, 3200.0f, 1200.0f);
TempHumiditySensor dht(4, 22);
LightSensor light(35);
MLLayer ml(MLBackend::TINYML_TFLM);
RecommendationEngine recEngine;
MonitoringSystem monitoringSystem(soil, dht, light, ml, recEngine, PlantRuleProfile{}, 24);

TimeService timeService;
WiFiCommunication wifiCommunication(WIFI_SSID, WIFI_PASSWORD, timeService);
FileStorageService fileStorageService(timeService);
WebServer server(80);
ThresholdWarningService thresholdWarningService;

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
    server.send(200, "text/plain", "Deleted " + filePath);
}

void HandleRoot()
{
    const PlantRuleProfile profile = monitoringSystem.GetPlantProfile();
    const auto thresholdWarnings = thresholdWarningService.BuildWarningsFromLog(fileStorageService.FindLatestLogFile(), profile);
    String html;
    html.reserve(18000);

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
    html += "<h2>Long-running threshold warnings</h2>";
    if (thresholdWarnings.empty())
    {
        html += "<p>No warnings for prolonged max-threshold exceedance in the last ~5 days.</p>";
    }
    else
    {
        html += "<ul>";
        for (const auto &warning : thresholdWarnings)
        {
            html += "<li><strong>" + warning.metric + ":</strong> above max threshold for about 5 days ";
            html += "(latest " + String(warning.latestValue, 1) + ", max " + String(warning.threshold, 1) + "). ";
            html += "Suggested action: " + warning.action + "</li>";
        }
        html += "</ul>";
    }

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

    html += "<hr><h2>Log files</h2><p>Total log files found: " + String(fileStorageService.CountLogFiles()) + "</p>";
    html += fileStorageService.BuildFilteredFilesHtmlTable(&FileStorageService::IsLogFile,
                                                           PLANT_SETTINGS_FILE,
                                                           LAST_STATE_FILE);
    html += "<p style='margin-top:16px;'><a href='/latest'>Download latest log</a></p>";
    html += "<p><a href='/list'>View file list as plain text</a></p>";

    html += "<script>";
    html += "async function loadSettings(){const r=await fetch('/api/plant-settings');const s=await r.json();const form=document.getElementById('settingsForm');form.plantName.value=s.plantName;form.humidityPreference.value=s.humidityPreference;form.temperaturePreference.value=s.temperaturePreference;form.soilPreference.value=s.soilPreference;form.lightPreference.value=s.lightPreference;}";
    html += "document.getElementById('settingsForm').addEventListener('submit', async (e)=>{e.preventDefault();const fd=new FormData(e.target);const body=new URLSearchParams(fd);const response=await fetch('/api/plant-settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});document.getElementById('result').textContent=await response.text();await loadSettings();});";
    html += "loadSettings();</script></body></html>";

    server.send(200, "text/html", html);
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

    String html;
    html.reserve(content.length() + 1200);
    html += "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>View file</title></head><body style='font-family:Arial,sans-serif;padding:20px;'>";
    html += "<h1>Viewing " + filePath + "</h1>";
    html += "<p><a href='/'>Back</a> | <a href='/download?file=" + rawPath + "'>Download</a></p>";
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
        file = root.openNextFile();
    }

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
    json += "\"plantName\":\"" + String(profile.plantName.c_str()) + "\",";
    json += "\"deviceId\":\"" + String(profile.deviceId.c_str()) + "\",";
    json += "\"deviceName\":\"" + String(profile.deviceName.c_str()) + "\",";
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
    server.on("/delete", HTTP_POST, HandleDelete);
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
    const auto result = monitoringSystem.RunCycleDetailed();
    fileStorageService.LogToCsv(result, PLANT_LABEL, DEVICE_NAME, DEVICE_ID, logCount);
    fileStorageService.SaveLastStateToFile(result, LAST_STATE_FILE);
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

    PlantRuleProfile defaultProfile;
    defaultProfile.plantName = PLANT_LABEL;
    defaultProfile.deviceName = DEVICE_NAME;
    defaultProfile.deviceId = String(DEVICE_ID).c_str();

    PlantRuleProfile activeProfile = fileStorageService.LoadPlantSettingsFromFile(PLANT_SETTINGS_FILE, defaultProfile);
    monitoringSystem.SetPlantProfile(activeProfile);

    if (!monitoringSystem.Init())
    {
        Serial.println("Failed to initialize monitoring system");
        while (true)
        {
        }
    }

    Serial.println("Monitoring system initialized");
    RunMonitoringCycle();

    const String latestFile = fileStorageService.FindLatestLogFile();
    if (latestFile.length() > 0)
    {
        fileStorageService.DumpFileToSerial(latestFile);
    }

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
