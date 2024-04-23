#pragma once
#include "IInputController.h"

class InputComponent
{
public:
	InputComponent(IInputController* inputController) : inputController(inputController) {};

	const IInputController* inputController;
};

