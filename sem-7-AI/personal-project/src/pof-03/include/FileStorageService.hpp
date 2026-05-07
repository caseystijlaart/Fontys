#pragma once

#include <Arduino.h>

#include "MonitoringSystem.hpp"

class TimeService;

class FileStorageService
{
public:
    explicit FileStorageService(TimeService &timeService);

    bool SaveLastStateToFile(const pof02::MonitoringCycleResult &result, const char *lastStateFile);
    bool LogToCsv(const pof02::MonitoringCycleResult &result, const char *plantLabel, const char *deviceName, int deviceId, unsigned long &logCount);

    bool SavePlantSettingsToFile(const pof02::PlantRuleProfile &profile, const char *plantSettingsFile);

    String FindLatestLogFile();
    void DumpFileToSerial(const String &filePath);

    String GetContentType(const String &path) const;
    bool IsSafePath(const String &path) const;
    String NormalizeFilePath(const String &path) const;
    bool IsViewableTextFile(const String &path) const;

    bool IsImportantFile(const String &path) const;
    bool IsLogFile(const String &path) const;
    size_t CountImportantFiles(const char *plantSettingsFile, const char *lastStateFile);
    size_t CountLogFiles();
    String BuildFilteredFilesHtmlTable(bool (FileStorageService::*filterFn)(const String &) const,
                                       const char *plantSettingsFile,
                                       const char *lastStateFile);

    void SetImportantFiles(const char *plantSettingsFile, const char *lastStateFile);

private:
    TimeService &timeService_;
    const char *plantSettingsFile_;
    const char *lastStateFile_;

    String PreferenceToString(pof02::PreferenceBand band) const;
    String EnsureLeadingSlash(const String &path) const;
    String GetDailyLogFilePath(std::int64_t unixTime) const;
    String CsvEscape(const String &input) const;
    bool EnsureCsvHeader(const String &filePath);
};
