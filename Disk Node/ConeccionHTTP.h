#pragma once
#include <string>
#include "Log.h"


class ConeccionHTTP
{
private:
	Log* logger;
public:
	void run(int port, std::string addr);
	bool ReadXML(const std::string& xmlFilePath, std::string& ip, int& port, std::string& path);
	ConeccionHTTP(Log* logPtr = nullptr) : logger(logPtr) {}
};

