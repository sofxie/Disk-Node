#pragma once
#include <string>
#include "Log.h"
#include "Include/json.hpp"
using json = nlohmann::json;


class ConeccionHTTP
{
private:
	Log* logger;
	std::string pathConfigurado;
	json rCommand(const json& j);
public:
	void run(int port, std::string addr);
	bool ReadXML(const std::string& xmlFilePath, std::string& ip, int& port, std::string& path);
	ConeccionHTTP(Log* logPtr = nullptr) : logger(logPtr) {}
};

