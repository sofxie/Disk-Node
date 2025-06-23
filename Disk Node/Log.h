#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <ctime>
#include <sstream>
#include <filesystem>

class Log {
private:
    std::string nodeName;
    std::vector<std::string> entries;
    std::mutex logMutex;
    std::string currentLogFile;
    const uint64_t MAX_FILE_SIZE = 104'857'600; // 100 MB en bytes

    std::string getTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm timeinfo;
        char buffer[80];

        localtime_s(&timeinfo, &now);
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return std::string(buffer);
    }

    std::string generateLogFileName() {
        std::time_t now = std::time(nullptr);
        std::tm timeinfo;
        char buffer[80];

        localtime_s(&timeinfo, &now);
        std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &timeinfo);
        return nodeName + "_log_" + buffer + ".txt";
    }

    bool isFileSizeLimitReached(const std::string& fileName) {
        try {
            if (std::filesystem::exists(fileName)) {
                return std::filesystem::file_size(fileName) >= MAX_FILE_SIZE;
            }
            return false;
        }
        catch (const std::filesystem::filesystem_error&) {
            return false; // Si hay error, asumimos que no se ha alcanzado el límite
        }
    }

    void openNewLogFile(std::ofstream& logFile) {
        if (logFile.is_open()) {
            logFile.close();
        }
        currentLogFile = generateLogFileName();
        logFile.open(currentLogFile, std::ios::app);
    }

public:
    Log(const std::string& name) : nodeName(name) {
        currentLogFile = generateLogFileName();
    }

    void addEntry(const std::string& entry) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::ostringstream logLine;
        logLine << "[" << getTimestamp() << "] " << entry;
        entries.push_back(logLine.str());

        // Verificar tamaño del archivo actual
        std::ofstream logFile(currentLogFile, std::ios::app);
        if (isFileSizeLimitReached(currentLogFile)) {
            openNewLogFile(logFile);
        }

        // Guardar en archivo
        if (logFile.is_open()) {
            logFile << logLine.str() << std::endl;
        }
    }

    std::vector<std::string> getEntries() {
        std::lock_guard<std::mutex> lock(logMutex);
        return entries;
    }
};

