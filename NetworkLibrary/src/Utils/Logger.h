#pragma once
#include <string>
#include <iostream>
#include <ctime>

#ifdef LOG_ENABLED
#define LOG_INFO(message) Logger::Info(message)
#define LOG_WARNING(message) Logger::Warning(message)
#define LOG_ERROR(message) Logger::Error(message)
#define TIME_FORMAT "%H:%M:%S"
#else
#define LOG_DEBUG(message)
#define LOG_WARNING(message)
#define LOG_ERROR(message)
#endif

class Logger
{
public:
	static void Info(const std::string& message)
	{
		Print("Info", message);
	}

	static void Warning(const std::string& message)
	{
		Print("Warn", message);
	}

	static void Error(const std::string& message)
	{
		Print("Error", message);
	}

private:
	static void Print(const std::string& prefix, const std::string& message)
	{
		std::time_t currentTime;
		std::time(&currentTime);
		std::tm timeInfo;
		localtime_s(&timeInfo, &currentTime);
		char timeBuffer[80];
		std::strftime(timeBuffer, sizeof(timeBuffer), TIME_FORMAT, &timeInfo);

		std::cout << "[" << timeBuffer << " | " << prefix << "]\t" << message << std::endl;
	}
};

