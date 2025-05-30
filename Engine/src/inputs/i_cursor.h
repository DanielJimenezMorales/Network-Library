#pragma once
#include <SDL.h>

namespace Engine
{
	class ICursor
	{
		public:
			virtual void HandleEvent( const SDL_Event& event ) = 0;
			virtual void ResetEvents() = 0;
			virtual void UpdateUnhandledButtons() = 0;
			virtual bool GetButtonDown( int32 actionId ) const = 0;
			virtual bool GetButtonPressed( int32 actionId ) const = 0;
			virtual bool GetButtonUp( int32 actionId ) const = 0;
			virtual void GetPosition( int32& x, int32& y ) const = 0;
			virtual void GetDelta( int32& x, int32& y ) const = 0;
	};
}
