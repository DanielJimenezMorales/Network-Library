#include "logger.h"
#include "numeric_types.h"
#include "dbg.h"

#include <cstdarg>
#include <string>

#ifdef DEBUG
namespace Common
{
	static constexpr char* INFO_PREFIX = "Info";
	static constexpr char* WARNING_PREFIX = "Warn";
	static constexpr char* ERROR_PREFIX = "Error";
	static constexpr char* FATAL_PREFIX = "Fatal";

	static constexpr char* RESET_COLOR_CODE = "\033[0m";
	static constexpr char* INFO_COLOR_CODE = "\033[37m";
	static constexpr char* WARNING_COLOR_CODE = "\033[33m";
	static constexpr char* ERROR_COLOR_CODE = "\033[91m";
	static constexpr char* FATAL_COLOR_CODE = "\033[91m";

	static void GetPrefixFromLevel( LogLevel level, std::string& prefix_buffer )
	{
		switch ( level )
		{
			case LogLevel::Info:
				prefix_buffer.assign( INFO_PREFIX );
				break;
			case LogLevel::Warning:
				prefix_buffer.assign( WARNING_PREFIX );
				break;
			case LogLevel::Error:
				prefix_buffer.assign( ERROR_PREFIX );
				break;
			case LogLevel::Fatal:
				prefix_buffer.assign( FATAL_PREFIX );
				break;
			default:
				prefix_buffer.assign( "UNKNOWN" );
				break;
		}
	}

	static void GetPrefixColorCodeFromLevel( LogLevel level, std::string& color_code_buffer )
	{
		switch ( level )
		{
			case LogLevel::Info:
				color_code_buffer.assign( INFO_COLOR_CODE );
				break;
			case LogLevel::Warning:
				color_code_buffer.assign( WARNING_COLOR_CODE );
				break;
			case LogLevel::Error:
				color_code_buffer.assign( ERROR_COLOR_CODE );
				break;
			case LogLevel::Fatal:
				color_code_buffer.assign( FATAL_COLOR_CODE );
				break;
			default:
				color_code_buffer.assign( RESET_COLOR_CODE );
				break;
		}
	}

	void Print( LogLevel level, const char* filePath, const char* line, const char* format, va_list args )
	{
		std::time_t currentTime;
		std::time( &currentTime );
		std::tm timeInfo;
		localtime_s( &timeInfo, &currentTime );
		char timeBuffer[ 80 ];
		std::strftime( timeBuffer, sizeof( timeBuffer ), TIME_FORMAT, &timeInfo );

		std::string prefix;
		GetPrefixFromLevel( level, prefix );
		std::string prefixColorCode;
		GetPrefixColorCodeFromLevel( level, prefixColorCode );
		printf( "%s[%s | %s]%s\t", prefixColorCode.c_str(), timeBuffer, prefix.c_str(), RESET_COLOR_CODE );
		vprintf( format, args );

		switch ( level )
		{
			case LogLevel::Error:
			case LogLevel::Fatal:
				uint8 numberOfLogFuncCallsToSkip = 2;
				const auto& stackTrace = dbg::stack_trace();
				for ( const auto& funcCall : stackTrace )
				{
					if ( numberOfLogFuncCallsToSkip > 0 )
					{
						numberOfLogFuncCallsToSkip--;
						continue;
					}
					printf( "\n\tat %s - %s:%d", funcCall.name.c_str(), funcCall.file.c_str(), funcCall.line );
				}
		}
		printf( "\n" );
	}

	void LogInfo( const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		Print( LogLevel::Info, '\0', '\0', format, args );
		va_end( args );
	}

	void LogWarning( const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		Print( LogLevel::Warning, '\0', '\0', format, args );
		va_end( args );
	}

	void LogError( const char* filePath, const char* line, const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		Print( LogLevel::Error, filePath, line, format, args );
		va_end( args );
	}

	void LogFatal( const char* filePath, const char* line, const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		Print( LogLevel::Fatal, filePath, line, format, args );
		va_end( args );
	}
} // namespace Common
#endif // DEBUG
