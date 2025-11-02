#pragma once

#ifdef DEBUG
	#define ASSERT( expression, text_format, ... )                         \
		if ( !( expression ) )                                             \
		{                                                                  \
			Common::ForceCrash( #expression, text_format, ##__VA_ARGS__ ); \
		}
#else
	#define ASSERT( expression, text_format, ... )
#endif
namespace Common
{
	void ForceCrash( const char* expression, const char* text_format, ... );
}