#pragma once

#ifdef DEBUG
	#if defined( _MSC_VER )
		#define DEBUG_BREAK() __debugbreak()
	#elif defined( __GNUC__ ) || defined( __clang__ )
		#include <signal.h>
		#define DEBUG_BREAK() raise( SIGTRAP )
	#else
	// raise(SIGTRAP) is like the most standard way to do a debug break in unix-like systems
		#include <signal.h>
		#define DEBUG_BREAK() raise( SIGTRAP )
	#endif

	#define ASSERT( expression, text_format, ... )                         \
		if ( !( expression ) )                                             \
		{                                                                  \
			Common::ForceCrash( #expression, text_format, ##__VA_ARGS__ ); \
		}
#else
	#define DEBUG_BREAK()
	#define ASSERT( expression, text_format, ... )
#endif
namespace Common
{
	void ForceCrash( const char* expression, const char* text_format, ... );
}