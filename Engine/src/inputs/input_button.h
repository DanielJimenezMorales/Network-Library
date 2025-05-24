#pragma once
#include "numeric_types.h"
#include "button_state.h"

namespace Engine
{
	struct InputButton
	{
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

			int32 actionId;
			int32 code;
			ButtonState currentState;
			ButtonState previousState;
			bool handledThisFrame;
	};
}
