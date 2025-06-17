#pragma once
#include <string>

class ConeccionHTTP
{
public:
	void run(int port, std::string addr);
	bool ReadXML(const std::string& xmlFilePath, std::string& ip, int& port, std::string& path);
};

