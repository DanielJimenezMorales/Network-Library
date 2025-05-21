#pragma once
#include <SDL.h>
#include <vector>

#include "numeric_types.h"
#include "logger.h"

namespace Engine
{
	class IInputController;
	class ICursor;

	enum class ButtonState
	{
		None,
		Pressed,
		Released
	};

	struct InputButton
	{
			int32 actionId;
			int32 code;
			ButtonState currentState;
			ButtonState previousState;
			bool handledThisFrame;

			InputButton()
			    : actionId( 0 )
			    , code( 0 )
			    , currentState( ButtonState::None )
			    , previousState( ButtonState::None )
			    , handledThisFrame( false )
			{
			}
			InputButton( int32 actionId, int32 code )
			    : actionId( actionId )
			    , code( code )
			    , currentState( ButtonState::None )
			    , previousState( ButtonState::None )
			    , handledThisFrame( false )
			{
			}
	};

	struct InputAxis
	{
			int32 actionId;
			int32 positiveCode;
			int32 negativeCode;
			ButtonState positiveState;
			ButtonState negativeState;
			float32 value;

			InputAxis()
			    : actionId( 0 )
			    , positiveCode( 0 )
			    , negativeCode( 0 )
			    , value( 0 )
			    , positiveState( ButtonState::None )
			    , negativeState( ButtonState::None )
			{
			}
			InputAxis( int32 actionId, int32 positiveCode, int32 negativeCode )
			    : actionId( actionId )
			    , positiveCode( positiveCode )
			    , negativeCode( negativeCode )
			    , value( 0 )
			    , positiveState( ButtonState::None )
			    , negativeState( ButtonState::None )
			{
			}
	};

	class InputHandler
	{
		public:
			void AddController( IInputController* inputController );
			void AddCursor( ICursor* cursor );
			void PreHandleEvents();
			void HandleEvent( const SDL_Event& event );
			void PostHandleEvents();

		private:
			std::vector< IInputController* > _controllers;
			std::vector< ICursor* > _cursors;
	};
} // namespace Engine
