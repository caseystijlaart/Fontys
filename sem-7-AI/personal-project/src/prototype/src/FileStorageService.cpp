#include "FileStorageService.hpp"

#include <FS.h>
#include <LittleFS.h>

#include <algorithm>
#include <ctime>
#include <vector>

#include "TimeService.hpp"

using namespace pof02;

namespace
{
String EnsureAbsolutePath(const String &path)
{
    if (path.startsWith("/"))
    {
        return path;
    }
    return "/" + path;
}

String JoinPath(const String &dir, const String &name)
{
    if (name.startsWith("/"))
    {
        return name;
    }

    if (dir == "/")
    {
        return "/" + name;
    }

    return dir + "/" + name;
}

String EntryNameOnly(const String &entryName)
{
    const int slashIndex = entryName.lastIndexOf('/');
    if (slashIndex >= 0)
    {
        return entryName.substring(slashIndex + 1);
    }
    return entryName;
}

std::vector<String> ListAllFilesRecursive()
{
    std::vector<String> files;
    std::vector<String> dirs{"/"};

    while (!dirs.empty())
    {
        const String dirPath = dirs.back();
        dirs.pop_back();

        File dir = LittleFS.open(dirPath, FILE_READ);
        if (!dir || !dir.isDirectory())
        {
            continue;
        }

        File entry = dir.openNextFile();
        while (entry)
        {
            const String entryName = EntryNameOnly(String(entry.name()));
            const String fullPath = JoinPath(dirPath, entryName);

            if (entry.isDirectory())
            {
                dirs.push_back(fullPath);
            }
            else
            {
                files.push_back(fullPath);
            }

            entry.close();
            entry = dir.openNextFile();
        }

        dir.close();
    }

    std::sort(files.begin(), files.end());
    return files;
}
}

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
    const String mlFilePath = GetMlLogFilePath(logUnixTime);

    if (!EnsureCsvHeader(filePath) || !EnsureCsvHeader(mlFilePath))
    {
        return false;
    }

    File file = LittleFS.open(filePath, FILE_APPEND);
    File mlFile = LittleFS.open(mlFilePath, FILE_APPEND);
    if (!file || !mlFile)
    {
        Serial.println("Failed to open log file(s) for append");
        if (file)
            file.close();
        if (mlFile)
            mlFile.close();
        return false;
    }

    String line;
    line.reserve(320);
    line += CsvEscape(String(plantLabel)) + ",";
    line += String(deviceId) + ",";
    line += CsvEscape(timestampLocal) + ",";
    line += String(snapshot.soilMoisturePct, 3) + ",";
    line += String(snapshot.temperatureC, 3) + ",";
    line += String(snapshot.humidityPct, 3) + ",";
    line += String(snapshot.lightLevelPct, 3) + ",";
    line += String(rec.water ? 1 : 0) + ",";
    line += String(rec.reduceTemp ? 1 : 0) + ",";
    line += String(rec.increaseLight ? 1 : 0) + ",";
    line += CsvEscape(String(rec.summary.c_str()));

    String mlLine;
    mlLine.reserve(220);
    mlLine += CsvEscape(String(plantLabel)) + ",";
    mlLine += CsvEscape(timestampLocal) + ",";
    mlLine += String(static_cast<int>(mlResult.risk)) + ",";
    mlLine += String(mlResult.confidence, 6) + ",";
    mlLine += String(mlResult.probabilities[0], 6) + ",";
    mlLine += String(mlResult.probabilities[1], 6) + ",";
    mlLine += String(mlResult.probabilities[2], 6);

    file.println(line);
    mlFile.println(mlLine);
    file.flush();
    mlFile.flush();
    file.close();
    mlFile.close();

    ++logCount;
    Serial.print("Logged row #");
    Serial.println(logCount);
    Serial.println(filePath);
    Serial.println(mlFilePath);

    return true;
}

std::vector<SensorSnapshot> FileStorageService::LoadRecentSnapshots(std::size_t maxEntries) const
{
    std::vector<SensorSnapshot> snapshots;
    String latest = const_cast<FileStorageService *>(this)->FindLatestLogFile();

    if (latest.length() == 0)
    {
        return snapshots;
    }

    File file = LittleFS.open(latest, FILE_READ);
    if (!file)
    {
        return snapshots;
    }

    std::vector<String> lines;
    while (file.available())
    {
        const String line = file.readStringUntil('\n');
        if (line.length() > 0)
        {
            lines.push_back(line);
        }
    }

    file.close();

    if (lines.size() <= 1)
    {
        return snapshots;
    }

    const std::size_t startIdx = lines.size() > maxEntries ? lines.size() - maxEntries : 1;

    for (std::size_t i = startIdx; i < lines.size(); ++i)
    {
        String line = lines[i];
        line.trim();

        if (line.length() == 0)
        {
            continue;
        }

        std::vector<String> parts;
        int from = 0;

        while (from >= 0)
        {
            const int comma = line.indexOf(',', from);
            if (comma < 0)
            {
                parts.push_back(line.substring(from));
                break;
            }

            parts.push_back(line.substring(from, comma));
            from = comma + 1;
        }

        if (parts.size() < 7)
        {
            continue;
        }

        SensorSnapshot s{};
        s.unixTime = 0;
        s.soilMoisturePct = parts[3].toFloat();
        s.temperatureC = parts[4].toFloat();
        s.humidityPct = parts[5].toFloat();
        s.lightLevelPct = parts[6].toFloat();

        snapshots.push_back(s);
    }

    return snapshots;
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

String FileStorageService::FindLatestLogFile()
{
    String latestName;

    for (const auto &name : ListAllFilesRecursive())
    {
        if (name.indexOf("/log_") >= 0 && name.endsWith(".csv") && name.indexOf("/ml_") < 0)
        {
            if (latestName.length() == 0 || name > latestName)
            {
                latestName = name;
            }
        }
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
        return "text/csv";
    if (path.endsWith(".txt"))
        return "text/plain";
    if (path.endsWith(".html"))
        return "text/html";
    if (path.endsWith(".json"))
        return "application/json";

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

    size_t count = 0;

    for (const auto &fileName : ListAllFilesRecursive())
    {
        if (IsImportantFile(fileName))
        {
            ++count;
        }
    }

    return count;
}

size_t FileStorageService::CountLogFiles()
{
    size_t count = 0;

    for (const auto &fileName : ListAllFilesRecursive())
    {
        if (IsLogFile(fileName))
        {
            ++count;
        }
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

    bool foundAny = false;
    const std::vector<String> allFiles = ListAllFilesRecursive();

    html += "<table border='1' cellpadding='6' cellspacing='0' style='border-collapse:collapse; width:100%; max-width:900px;'>";
    html += "<thead><tr><th align='left'>File</th><th align='left'>Size (bytes)</th><th align='left'>Actions</th></tr></thead><tbody>";

    for (const auto &name : allFiles)
    {
        if ((this->*filterFn)(name))
        {
            foundAny = true;

            String normalizedName = name;
            if (normalizedName.startsWith("/"))
            {
                normalizedName = normalizedName.substring(1);
            }

            html += "<tr><td><code>" + name + "</code></td>";

            File file = LittleFS.open(EnsureAbsolutePath(name), FILE_READ);
            html += "<td>" + String(file ? file.size() : 0) + "</td><td>";
            if (file)
            {
                file.close();
            }

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
    }

    html += "</tbody></table>";

    if (!foundAny)
    {
        return "<p>No matching files found.</p>";
    }

    return html;
}

String FileStorageService::BuildFolderBrowserHtmlTable(const String &currentDir,
                                                       bool (FileStorageService::*filterFn)(const String &) const,
                                                       const char *plantSettingsFile,
                                                       const char *lastStateFile)
{
    SetImportantFiles(plantSettingsFile, lastStateFile);

    String dirPath = NormalizeFilePath(currentDir);
    if (dirPath.length() == 0)
    {
        dirPath = "/logs";
    }

    if (!IsSafePath(dirPath) || !dirPath.startsWith("/logs"))
    {
        return "<p>Invalid folder path.</p>";
    }

    File dir = LittleFS.open(dirPath, FILE_READ);
    if (!dir || !dir.isDirectory())
    {
        return "<p>Folder not found: <code>" + dirPath + "</code></p>";
    }

    String html;
    html.reserve(7000);

    html += "<p>Current folder: <code>" + dirPath + "</code></p>";

    if (dirPath != "/logs")
    {
        const int lastSlash = dirPath.lastIndexOf('/');
        String parent = lastSlash <= 0 ? "/logs" : dirPath.substring(0, lastSlash);

        if (parent.length() == 0 || parent == "/" || !parent.startsWith("/logs"))
        {
            parent = "/logs";
        }

        html += "<p><a href='/?dir=" + parent + "'>Back</a></p>";
    }

    html += "<table border='1' cellpadding='6' cellspacing='0' style='border-collapse:collapse; width:100%; max-width:900px;'>";
    html += "<thead><tr><th align='left'>Name</th><th align='left'>Type</th><th align='left'>Size</th><th align='left'>Actions</th></tr></thead><tbody>";

    bool foundAny = false;

    File entry = dir.openNextFile();
    while (entry)
    {
        const String entryName = EntryNameOnly(String(entry.name()));
        const String fullPath = JoinPath(dirPath, entryName);

        if (entry.isDirectory())
        {
            foundAny = true;

            html += "<tr>";
            html += "<td><code>" + entryName + "</code></td>";
            html += "<td>Folder</td>";
            html += "<td>-</td>";
            html += "<td>";
            html += "<a href='/?dir=" + fullPath + "'>Open</a>";

            if (fullPath != "/logs")
            {
                html += " | <form method='POST' action='/delete-folder' style='display:inline; margin:0;'>";
                html += "<input type='hidden' name='folder' value='" + fullPath + "'>";
                html += "<button type='submit' onclick='return confirm(\"Delete this folder and ALL contents?\")'>Delete folder</button>";
                html += "</form>";
            }

            html += "</td>";
            html += "</tr>";
        }
        else if ((this->*filterFn)(fullPath))
        {
            foundAny = true;

            String normalizedName = fullPath;
            if (normalizedName.startsWith("/"))
            {
                normalizedName = normalizedName.substring(1);
            }

            html += "<tr>";
            html += "<td><code>" + entryName + "</code></td>";
            html += "<td>File</td>";
            html += "<td>" + String(entry.size()) + "</td>";
            html += "<td>";

            if (IsViewableTextFile(fullPath))
            {
                html += "<a href='/view?file=" + normalizedName + "'>View</a> | ";
            }

            html += "<a href='/download?file=" + normalizedName + "'>Download</a>";

            if (!IsImportantFile(fullPath))
            {
                html += " | <form method='POST' action='/delete' style='display:inline; margin:0;'>";
                html += "<input type='hidden' name='file' value='" + normalizedName + "'>";
                html += "<button type='submit' onclick='return confirm(\"Delete this file?\")'>Delete</button>";
                html += "</form>";
            }

            html += "</td></tr>";
        }

        entry.close();
        entry = dir.openNextFile();
    }

    dir.close();

    html += "</tbody></table>";

    if (!foundAny)
    {
        html += "<p>No matching files found in this folder.</p>";
    }

    return html;
}

bool FileStorageService::DeleteFolderRecursive(const String &folderPath)
{
    const String dirPath = NormalizeFilePath(folderPath);

    if (!IsSafePath(dirPath) || !dirPath.startsWith("/logs") || dirPath == "/logs")
    {
        Serial.print("Refusing to delete folder: ");
        Serial.println(dirPath);
        return false;
    }

    File dir = LittleFS.open(dirPath, FILE_READ);
    if (!dir || !dir.isDirectory())
    {
        Serial.print("Folder not found or not directory: ");
        Serial.println(dirPath);
        return false;
    }

    File entry = dir.openNextFile();
    while (entry)
    {
        const String entryName = EntryNameOnly(String(entry.name()));
        const String fullPath = JoinPath(dirPath, entryName);

        if (entry.isDirectory())
        {
            entry.close();

            if (!DeleteFolderRecursive(fullPath))
            {
                dir.close();
                return false;
            }
        }
        else
        {
            entry.close();

            if (!LittleFS.remove(fullPath))
            {
                Serial.print("Failed to delete file: ");
                Serial.println(fullPath);
                dir.close();
                return false;
            }

            Serial.print("Deleted file: ");
            Serial.println(fullPath);
        }

        entry = dir.openNextFile();
    }

    dir.close();

    if (!LittleFS.rmdir(dirPath))
    {
        Serial.print("Failed to remove folder: ");
        Serial.println(dirPath);
        return false;
    }

    Serial.print("Deleted folder: ");
    Serial.println(dirPath);
    return true;
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

String FileStorageService::GetMonthlyDirectoryPath(std::int64_t unixTime) const
{
    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[24];
    if (std::strftime(buffer, sizeof(buffer), "/logs/%Y-%m", &timeInfo) == 0)
    {
        return "/logs/unknown";
    }

    return String(buffer);
}

String FileStorageService::GetDailyLogFilePath(std::int64_t unixTime) const
{
    const String monthDir = GetMonthlyDirectoryPath(unixTime);

    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[32];
    if (std::strftime(buffer, sizeof(buffer), "log_%Y-%m-%d.csv", &timeInfo) == 0)
    {
        return monthDir + "/log_unknown_date.csv";
    }

    return monthDir + "/" + String(buffer);
}

String FileStorageService::GetMlLogFilePath(std::int64_t unixTime) const
{
    const String monthDir = GetMonthlyDirectoryPath(unixTime);

    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[32];
    if (std::strftime(buffer, sizeof(buffer), "ml_%Y-%m-%d.csv", &timeInfo) == 0)
    {
        return monthDir + "/ml_unknown_date.csv";
    }

    return monthDir + "/" + String(buffer);
}

bool FileStorageService::EnsureDirectoryExists(const String &dirPath) const
{
    if (LittleFS.exists(dirPath))
    {
        return true;
    }

    Serial.print("Creating directory: ");
    Serial.println(dirPath);

    if (!LittleFS.mkdir(dirPath))
    {
        Serial.print("Failed to create directory: ");
        Serial.println(dirPath);
        return false;
    }

    return true;
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

    const int slashIndex = filePath.lastIndexOf('/');
    if (slashIndex > 0)
    {
        const String dirPath = filePath.substring(0, slashIndex);
        const int parentSlash = dirPath.lastIndexOf('/');

        if (parentSlash > 0)
        {
            const String parentDir = dirPath.substring(0, parentSlash);

            if (!EnsureDirectoryExists(parentDir))
            {
                return false;
            }
        }

        if (!EnsureDirectoryExists(dirPath))
        {
            return false;
        }
    }

    File file = LittleFS.open(filePath, FILE_WRITE);
    if (!file)
    {
        Serial.print("Failed to create CSV file: ");
        Serial.println(filePath);
        return false;
    }

    if (filePath.indexOf("/ml_") >= 0)
    {
        file.println("plant_label,timestamp_utc,risk_class,confidence,prob_healthy,prob_moderate_stress,prob_high_stress");
    }
    else
    {
        file.println("plant_label,device_id,timestamp_utc,soil_moisture_pct,temperature_c,humidity_pct,light_level_pct,action_water,action_reduce_temp,action_increase_light,recommendation_summary");
    }

    file.close();

    Serial.print("Created CSV with header: ");
    Serial.println(filePath);
    return true;
}
