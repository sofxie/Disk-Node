#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <ctime>
#include <sstream>

class Log {
private:
    std::string nodeName;
    std::vector<std::string> entries;
    std::mutex logMutex;

    std::string getTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm* timeinfo = std::localtime(&now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }

public:
    Log(const std::string& name) : nodeName(name) {}

    void addEntry(const std::string& entry) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::ostringstream logLine;
        logLine << "[" << getTimestamp() << "] " << entry;
        entries.push_back(logLine.str());

        // Guardar en archivo
        std::ofstream logFile(nodeName + "_log.txt", std::ios::app);
        if (logFile.is_open()) {
            logFile << logLine.str() << std::endl;
        }
    }

    std::vector<std::string> getEntries() {
        std::lock_guard<std::mutex> lock(logMutex);
        return entries;
    }
};


