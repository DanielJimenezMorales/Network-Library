#pragma once

class IInputController;

class InputComponent
{
public:
	InputComponent(IInputController* inputController) : inputController(inputController) {};

	const IInputController* inputController;
};
