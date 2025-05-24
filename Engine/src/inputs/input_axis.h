#pragma once
#include "numeric_types.h"
#include "button_state.h"

namespace Engine
{
	struct InputAxis
	{
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

			int32 actionId;
			int32 positiveCode;
			int32 negativeCode;
			ButtonState positiveState;
			ButtonState negativeState;
			float32 value;
	};
}
