#pragma once

class IInputController;
class ICursor;

struct InputComponent
{
	public:
		InputComponent()
		    : inputController( nullptr )
		    , cursor( nullptr )
		{
		}

		InputComponent( IInputController* inputController, ICursor* cursor )
		    : inputController( inputController )
		    , cursor( cursor )
		{
		}

		IInputController* inputController;
		ICursor* cursor;
};
