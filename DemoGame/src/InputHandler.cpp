#include "InputHandler.h"
#include "IInputController.h"

void InputHandler::AddController(IInputController* inputController)
{
	_controllers.push_back(inputController);
}

void InputHandler::PreHandleEvents()
{
	for (const auto& controller : _controllers)
	{
		controller->ResetEvents();
	}
}

void InputHandler::HandleEvent(const SDL_Event& event)
{
	for (const auto& controller : _controllers)
	{
		controller->HandleEvent(event);
	}
}

void InputHandler::PostHandleEvents()
{
	for (const auto& controller : _controllers)
	{
		controller->UpdateUnhandledButtons();
	}
}
