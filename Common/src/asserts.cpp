#include "asserts.h"
#include "logger.h"

#include <string>
#include <windows.h>
#include <vector>

namespace Common
{
	void ForceCrash( const char* expression, const char* text_format, ... )
	{
		std::string message;
		message.append( "ASSERT failed while evaluating expression: " );
		message.append(expression );
		message.append(". ");

		std::vector< char > buffer( 1024 );
		va_list args;
		va_start( args, text_format );
		vsnprintf( buffer.data(), buffer.size(), text_format, args );
		va_end( args );

		message.append( buffer.data() );
		LOG_FATAL( message.c_str() );

		// Try to break if a debugger exists and then exit the application
		if ( IsDebuggerPresent() )
		{
			DEBUG_BREAK();
		}
		exit( -1 );
	}
}
