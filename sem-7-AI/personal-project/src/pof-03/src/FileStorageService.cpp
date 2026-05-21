#include "FileStorageService.hpp"

#include <FS.h>
#include <LittleFS.h>

#include <ctime>

#include "TimeService.hpp"

using namespace pof02;

FileStorageService::FileStorageService(TimeService &timeService)
    : timeService_(timeService),
      plantSettingsFile_("/plant_settings.txt"),
      lastStateFile_("/last_state.txt")
{
}

bool FileStorageService::SaveLastStateToFile(const MonitoringCycleResult &result, const char *lastStateFile)
{
    File file = LittleFS.open(lastStateFile, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open last_state.txt for writing");
        return false;
    }

    std::int64_t unixTime = timeService_.GetCurrentUnixTimeUtc();
    if (unixTime == 0)
    {
        unixTime = result.snapshot.unixTime;
    }

    file.println("timestamp_local=" + timeService_.FormatTimestampLocal(unixTime));
    file.println("soil_moisture_pct=" + String(result.snapshot.soilMoisturePct, 3));
    file.println("temperature_c=" + String(result.snapshot.temperatureC, 3));
    file.println("humidity_pct=" + String(result.snapshot.humidityPct, 3));
    file.println("light_level_pct=" + String(result.snapshot.lightLevelPct, 3));
    file.println("risk_class=" + String(static_cast<int>(result.mlResult.risk)));
    file.println("confidence=" + String(result.mlResult.confidence, 6));
    file.println("prob_healthy=" + String(result.mlResult.probabilities[0], 6));
    file.println("prob_moderate_stress=" + String(result.mlResult.probabilities[1], 6));
    file.println("prob_high_stress=" + String(result.mlResult.probabilities[2], 6));
    file.println("action_water=" + String(result.recommendation.water ? 1 : 0));
    file.println("action_reduce_temp=" + String(result.recommendation.reduceTemp ? 1 : 0));
    file.println("action_increase_humidity=" + String(result.recommendation.increaseHumidity ? 1 : 0));
    file.println("action_increase_light=" + String(result.recommendation.increaseLight ? 1 : 0));
    file.println("recommendation_summary=" + String(result.recommendation.summary.c_str()));

    file.close();
    Serial.println("Saved /last_state.txt");
    return true;
}

bool FileStorageService::LogToCsv(const MonitoringCycleResult &result,
                                  const char *plantLabel,
                                  const char *deviceName,
                                  int deviceId,
                                  unsigned long &logCount)
{
    const auto &snapshot = result.snapshot;
    const auto &mlResult = result.mlResult;
    const auto &rec = result.recommendation;

    std::int64_t logUnixTime = snapshot.unixTime;
    const std::int64_t syncedUnixTime = timeService_.GetCurrentUnixTimeUtc();
    if (syncedUnixTime > 0)
    {
        logUnixTime = syncedUnixTime;
    }

    const String timestampLocal = timeService_.FormatTimestampLocal(logUnixTime);
    const String filePath = GetDailyLogFilePath(logUnixTime);

    if (!EnsureCsvHeader(filePath))
    {
        return false;
    }

    File file = LittleFS.open(filePath, FILE_APPEND);
    if (!file)
    {
        Serial.print("Failed to open CSV file for append: ");
        Serial.println(filePath);
        return false;
    }

    String line;
    line.reserve(512);

    line += CsvEscape(String(plantLabel)) + ",";
    line += CsvEscape(String(deviceName)) + ",";
    line += String(deviceId) + ",";
    line += CsvEscape(timestampLocal) + ",";
    line += String(static_cast<long long>(logUnixTime)) + ",";
    line += String(snapshot.soilMoisturePct, 3) + ",";
    line += String(snapshot.temperatureC, 3) + ",";
    line += String(snapshot.humidityPct, 3) + ",";
    line += String(snapshot.lightLevelPct, 3) + ",";
    line += String(static_cast<int>(mlResult.risk)) + ",";
    line += String(mlResult.confidence, 6) + ",";
    line += String(mlResult.probabilities[0], 6) + ",";
    line += String(mlResult.probabilities[1], 6) + ",";
    line += String(mlResult.probabilities[2], 6) + ",";
    line += String(rec.water ? 1 : 0) + ",";
    line += String(rec.reduceTemp ? 1 : 0) + ",";
    line += String(rec.increaseHumidity ? 1 : 0) + ",";
    line += String(rec.increaseLight ? 1 : 0) + ",";
    line += CsvEscape(String(rec.summary.c_str()));

    file.println(line);
    file.flush();
    const size_t currentSize = file.size();
    file.close();

    ++logCount;
    Serial.print("Logged row #");
    Serial.println(logCount);
    Serial.print("Written to: ");
    Serial.println(filePath);
    Serial.println(line);
    Serial.print("File size: ");
    Serial.println(currentSize);

    return true;
}

bool FileStorageService::SavePlantSettingsToFile(const PlantRuleProfile &profile, const char *plantSettingsFile)
{
    File file = LittleFS.open(plantSettingsFile, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open plant_settings.txt for writing");
        return false;
    }

    file.println("plantName=" + String(profile.plantName.c_str()));
    file.println("deviceName=" + String(profile.deviceName.c_str()));
    file.println("deviceId=" + String(profile.deviceId.c_str()));
    file.println("soilPreference=" + PreferenceToString(profile.preferences.soilMoisture));
    file.println("temperaturePreference=" + PreferenceToString(profile.preferences.temperature));
    file.println("humidityPreference=" + PreferenceToString(profile.preferences.humidity));
    file.println("lightPreference=" + PreferenceToString(profile.preferences.light));

    file.close();
    Serial.println("Saved /plant_settings.txt");
    return true;
}

PlantRuleProfile FileStorageService::LoadPlantSettingsFromFile(const char *plantSettingsFile, const PlantRuleProfile &fallbackProfile)
{
    if (!LittleFS.exists(plantSettingsFile))
    {
        return fallbackProfile;
    }

    File file = LittleFS.open(plantSettingsFile, FILE_READ);
    if (!file)
    {
        return fallbackProfile;
    }

    PlantRuleProfile loaded = fallbackProfile;
    while (file.available())
    {
        const String line = file.readStringUntil('\n');
        const int equalsIndex = line.indexOf('=');
        if (equalsIndex <= 0)
        {
            continue;
        }

        const String key = line.substring(0, equalsIndex);
        String value = line.substring(equalsIndex + 1);
        value.trim();

        if (key == "plantName")
        {
            loaded.plantName = value.c_str();
        }
        else if (key == "deviceName")
        {
            loaded.deviceName = value.c_str();
        }
        else if (key == "deviceId")
        {
            loaded.deviceId = value.c_str();
        }
        else if (key == "soilPreference")
        {
            loaded.preferences.soilMoisture = value.equalsIgnoreCase("pLow") || value.equalsIgnoreCase("low") ? PreferenceBand::pLow : value.equalsIgnoreCase("pHigh") || value.equalsIgnoreCase("high") ? PreferenceBand::pHigh
                                                                                                                                                                                                     : PreferenceBand::pMid;
        }
        else if (key == "temperaturePreference")
        {
            loaded.preferences.temperature = value.equalsIgnoreCase("pLow") || value.equalsIgnoreCase("low") ? PreferenceBand::pLow : value.equalsIgnoreCase("pHigh") || value.equalsIgnoreCase("high") ? PreferenceBand::pHigh
                                                                                                                                                                                                   : PreferenceBand::pMid;
        }
        else if (key == "humidityPreference")
        {
            loaded.preferences.humidity = value.equalsIgnoreCase("pLow") || value.equalsIgnoreCase("low") ? PreferenceBand::pLow : value.equalsIgnoreCase("pHigh") || value.equalsIgnoreCase("high") ? PreferenceBand::pHigh
                                                                                                                                                                                                : PreferenceBand::pMid;
        }
        else if (key == "lightPreference")
        {
            loaded.preferences.light = value.equalsIgnoreCase("pLow") || value.equalsIgnoreCase("low") ? PreferenceBand::pLow : value.equalsIgnoreCase("pHigh") || value.equalsIgnoreCase("high") ? PreferenceBand::pHigh
                                                                                                                                                                                             : PreferenceBand::pMid;
        }
    }

    file.close();
    return loaded;
}

String FileStorageService::FindLatestLogFile()
{
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        return "";
    }

    String latestName;
    File file = root.openNextFile();
    while (file)
    {
        const String name = String(file.name());
        if (name.startsWith("/log_") && name.endsWith(".csv"))
        {
            if (latestName.length() == 0 || name > latestName)
            {
                latestName = name;
            }
        }
        file = root.openNextFile();
    }

    return latestName;
}

void FileStorageService::DumpFileToSerial(const String &filePath)
{
    File file = LittleFS.open(filePath, FILE_READ);
    if (!file)
    {
        Serial.print("Failed to open file for reading: ");
        Serial.println(filePath);
        return;
    }

    Serial.print("---- FILE START ");
    Serial.print(filePath);
    Serial.println(" ----");

    while (file.available())
    {
        Serial.write(file.read());
    }

    Serial.println();
    Serial.print("---- FILE END ");
    Serial.print(filePath);
    Serial.println(" ----");

    file.close();
}

String FileStorageService::GetContentType(const String &path) const
{
    if (path.endsWith(".csv"))
    {
        return "text/csv";
    }
    if (path.endsWith(".txt"))
    {
        return "text/plain";
    }
    if (path.endsWith(".html"))
    {
        return "text/html";
    }
    if (path.endsWith(".json"))
    {
        return "application/json";
    }
    return "application/octet-stream";
}

bool FileStorageService::IsSafePath(const String &path) const
{
    if (path.length() == 0)
    {
        return false;
    }

    return path.indexOf("..") < 0;
}

String FileStorageService::NormalizeFilePath(const String &path) const
{
    if (path.length() == 0)
    {
        return "";
    }

    if (path.startsWith("/"))
    {
        return path;
    }

    return "/" + path;
}

bool FileStorageService::IsViewableTextFile(const String &path) const
{
    return path.endsWith(".csv") ||
           path.endsWith(".txt") ||
           path.endsWith(".json") ||
           path.endsWith(".html");
}

bool FileStorageService::IsImportantFile(const String &path) const
{
    const String normalized = EnsureLeadingSlash(path);
    return normalized == plantSettingsFile_ || normalized == lastStateFile_;
}

bool FileStorageService::IsLogFile(const String &path) const
{
    const String normalized = EnsureLeadingSlash(path);
    return normalized.endsWith(".csv");
}

size_t FileStorageService::CountImportantFiles(const char *plantSettingsFile, const char *lastStateFile)
{
    SetImportantFiles(plantSettingsFile, lastStateFile);

    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        return 0;
    }

    size_t count = 0;
    File file = root.openNextFile();
    while (file)
    {
        if (IsImportantFile(String(file.name())))
        {
            ++count;
        }
        file = root.openNextFile();
    }
    return count;
}

size_t FileStorageService::CountLogFiles()
{
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        return 0;
    }

    size_t count = 0;
    File file = root.openNextFile();
    while (file)
    {
        if (IsLogFile(String(file.name())))
        {
            ++count;
        }
        file = root.openNextFile();
    }
    return count;
}

String FileStorageService::BuildFilteredFilesHtmlTable(bool (FileStorageService::*filterFn)(const String &) const,
                                                       const char *plantSettingsFile,
                                                       const char *lastStateFile)
{
    SetImportantFiles(plantSettingsFile, lastStateFile);

    String html;
    html.reserve(5000);

    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        html += "<p>Could not open LittleFS root directory.</p>";
        return html;
    }

    File file = root.openNextFile();
    bool foundAny = false;

    html += "<table border='1' cellpadding='6' cellspacing='0' style='border-collapse:collapse; width:100%; max-width:900px;'>";
    html += "<thead><tr><th align='left'>File</th><th align='left'>Size (bytes)</th><th align='left'>Actions</th></tr></thead><tbody>";

    while (file)
    {
        const String name = String(file.name());

        if ((this->*filterFn)(name))
        {
            foundAny = true;
            String normalizedName = name;
            if (normalizedName.startsWith("/"))
            {
                normalizedName = normalizedName.substring(1);
            }

            html += "<tr><td><code>" + name + "</code></td>";
            html += "<td>" + String(file.size()) + "</td><td>";

            if (IsViewableTextFile(name))
            {
                html += "<a href='/view?file=" + normalizedName + "'>View</a> | ";
            }

            html += "<a href='/download?file=" + normalizedName + "'>Download</a>";

            if (!IsImportantFile(name))
            {
                html += " | <form method='POST' action='/delete' style='display:inline; margin:0;'>";
                html += "<input type='hidden' name='file' value='" + normalizedName + "'>";
                html += "<button type='submit' onclick='return confirm(\"Delete this file?\")'>Delete</button>";
                html += "</form>";
            }

            html += "</td></tr>";
        }

        file = root.openNextFile();
    }

    html += "</tbody></table>";

    if (!foundAny)
    {
        return "<p>No matching files found.</p>";
    }

    return html;
}

void FileStorageService::SetImportantFiles(const char *plantSettingsFile, const char *lastStateFile)
{
    plantSettingsFile_ = plantSettingsFile;
    lastStateFile_ = lastStateFile;
}

String FileStorageService::PreferenceToString(PreferenceBand band) const
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

String FileStorageService::EnsureLeadingSlash(const String &path) const
{
    if (path.startsWith("/"))
    {
        return path;
    }
    return "/" + path;
}

String FileStorageService::GetDailyLogFilePath(std::int64_t unixTime) const
{
    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[32];
    const std::size_t len = std::strftime(buffer, sizeof(buffer), "/log_%Y-%m-%d.csv", &timeInfo);
    if (len == 0)
    {
        return "/log_unknown_date.csv";
    }

    return String(buffer);
}

String FileStorageService::CsvEscape(const String &input) const
{
    String escaped = input;
    escaped.replace("\"", "\"\"");
    return "\"" + escaped + "\"";
}

bool FileStorageService::EnsureCsvHeader(const String &filePath)
{
    if (LittleFS.exists(filePath))
    {
        return true;
    }

    File file = LittleFS.open(filePath, FILE_WRITE);
    if (!file)
    {
        Serial.print("Failed to create CSV file: ");
        Serial.println(filePath);
        return false;
    }

    file.println(
        "plant_label,device_name,device_id,timestamp_utc,unix_time,"
        "soil_moisture_pct,temperature_c,humidity_pct,light_level_pct,"
        "risk_class,confidence,prob_healthy,prob_moderate_stress,prob_high_stress,"
        "action_water,action_reduce_temp,action_increase_humidity,action_increase_light,"
        "recommendation_summary");

    file.close();

    Serial.print("Created CSV with header: ");
    Serial.println(filePath);
    return true;
}
