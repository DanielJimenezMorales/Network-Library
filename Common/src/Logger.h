#pragma once
#include <string>
#include <iostream>
#include <ctime>

#ifdef LOG_ENABLED

//For some reason I need to create two macros in order to make it work
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define LOG_INFO(message, ...) Common::LogInfo(message, __VA_ARGS__)
#define LOG_WARNING(message, ...) Common::LogWarning(message, __VA_ARGS__)
#define LOG_ERROR(message, ...) Common::LogError(__FILE__, STRINGIFY(__LINE__), message, __VA_ARGS__)
#define TIME_FORMAT "%H:%M:%S"

static constexpr char* INFO_PREFIX = "Info";
static constexpr char* WARNING_PREFIX = "Warn";
static constexpr char* ERROR_PREFIX = "Error";

static constexpr char* RESET_COLOR_CODE = "\033[0m";
static constexpr char* INFO_COLOR_CODE = "\033[37m";
static constexpr char* WARNING_COLOR_CODE = "\033[33m";
static constexpr char* ERROR_COLOR_CODE = "\033[91m";

namespace Common
{
	void Print(const char* prefix, const char* prefixColorCode, const char* filePath, const char* line, const char* format, va_list args);
	void LogInfo(const char* format, ...);
	void LogWarning(const char* format, ...);
	void LogError(const char* filePath, const char* line, const char* format, ...);
}
#else
#define LOG_INFO(message, ...)
#define LOG_WARNING(message, ...)
#define LOG_ERROR(message, ...)
#endif
