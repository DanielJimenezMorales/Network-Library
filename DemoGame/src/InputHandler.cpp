#include "InputHandler.h"
#include "IInputController.h"
#include "ICursor.h"

void InputHandler::AddController(IInputController* inputController)
{
	_controllers.push_back(inputController);
}

void InputHandler::AddCursor(ICursor* cursor)
{
	_cursors.push_back(cursor);
}

void InputHandler::PreHandleEvents()
{
	for (const auto& controller : _controllers)
	{
		controller->ResetEvents();
	}

	for (const auto& cursor : _cursors)
	{
		cursor->ResetEvents();
	}
}

void InputHandler::HandleEvent(const SDL_Event& event)
{
	for (const auto& controller : _controllers)
	{
		controller->HandleEvent(event);
	}

	for (const auto& cursor : _cursors)
	{
		cursor->HandleEvent(event);
	}
}

void InputHandler::PostHandleEvents()
{
	for (const auto& controller : _controllers)
	{
		controller->UpdateUnhandledButtons();
	}

	for (const auto& cursor : _cursors)
	{
		cursor->UpdateUnhandledButtons();
	}
}
