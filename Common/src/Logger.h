#pragma once
#include <iostream>
#include <ctime>

#ifdef DEBUG
	#define THIS_FUNCTION_NAME __func__

    // For some reason I need to create two macros in order to make it work
	#define STRINGIFY2( x ) #x
	#define STRINGIFY( x ) STRINGIFY2( x )

	#define LOG_INFO( message, ... ) Common::LogInfo( message, __VA_ARGS__ )
	#define LOG_WARNING( message, ... ) Common::LogWarning( message, __VA_ARGS__ )
	#define LOG_ERROR( message, ... ) Common::LogError( __FILE__, STRINGIFY( __LINE__ ), message, __VA_ARGS__ )
	#define LOG_FATAL( message, ... ) Common::LogFatal( __FILE__, STRINGIFY( __LINE__ ), message, __VA_ARGS__ )
	#define TIME_FORMAT "%H:%M:%S"

namespace Common
{
	enum class LogLevel
	{
		Info = 0,
		Warning = 1,
		Error = 2,
		Fatal = 3
	};

	void Print( LogLevel level, const char* filePath, const char* line, const char* format, va_list args );
	void LogInfo( const char* format, ... );
	void LogWarning( const char* format, ... );
	void LogError( const char* filePath, const char* line, const char* format, ... );
	void LogFatal( const char* filePath, const char* line, const char* format, ... );
} // namespace Common
#else
	#define THIS_FUNCTION_NAME "N/A"

	#define LOG_INFO( message, ... )
	#define LOG_WARNING( message, ... )
	#define LOG_ERROR( message, ... )
	#define LOG_FATAL( message, ... )
#endif