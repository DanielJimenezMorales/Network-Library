#pragma once

namespace Engine
{
	class IInputController;
	class ICursor;
}

struct InputComponent
{
	public:
		InputComponent()
		    : inputController( nullptr )
		    , cursor( nullptr )
		{
		}

		InputComponent( Engine::IInputController* inputController, Engine::ICursor* cursor )
		    : inputController( inputController )
		    , cursor( cursor )
		{
		}

		Engine::IInputController* inputController;
		Engine::ICursor* cursor;
};
