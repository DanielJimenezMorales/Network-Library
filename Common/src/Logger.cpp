#include "logger.h"

#include <cstdarg>

#ifdef LOG_ENABLED
namespace Common
{
	void Print(const char* prefix, const char* prefixColorCode, const char* filePath, const char* line, const char* format, va_list args)
	{
		std::time_t currentTime;
		std::time(&currentTime);
		std::tm timeInfo;
		localtime_s(&timeInfo, &currentTime);
		char timeBuffer[80];
		std::strftime(timeBuffer, sizeof(timeBuffer), TIME_FORMAT, &timeInfo);

		printf("%s[%s | %s]%s\t", prefixColorCode, timeBuffer, prefix, RESET_COLOR_CODE);
		vprintf(format, args);
		if (filePath != "" && line != "")
		{
			printf(" at %s:%s", filePath, line);
		}
		printf("\n");
	}

	void LogInfo(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Print(INFO_PREFIX, INFO_COLOR_CODE, "", "", format, args);
		va_end(args);
	}

	void LogWarning(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Print(WARNING_PREFIX, WARNING_COLOR_CODE, "", "", format, args);
		va_end(args);
	}

	void LogError(const char* filePath, const char* line, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		Print(ERROR_PREFIX, ERROR_COLOR_CODE, filePath, line, format, args);
		va_end(args);
	}
}
#endif // LOG_ENABLED