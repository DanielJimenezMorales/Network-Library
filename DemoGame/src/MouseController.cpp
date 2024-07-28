#include "MouseController.h"
#include "Logger.h"

void MouseController::AddButtonMap(const InputButton& inputButton)
{
	_actionToButtonMap[inputButton.actionId] = inputButton;
	_keyToButtonActionMap[static_cast<uint8_t>(inputButton.code)] = inputButton.actionId;
}

void MouseController::HandleEvent(const SDL_Event& event)
{
	if (event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEBUTTONUP)
	{
		return;
	}

	HandleButton(event);
}

void MouseController::ResetEvents()
{
	for (auto& item : _actionToButtonMap)
	{
		if (item.second.currentState == ButtonState::Released)
		{
			SetInputButtonState(item.second, ButtonState::None);
		}

		item.second.handledThisFrame = false;
	}
}

void MouseController::UpdateUnhandledButtons()
{
	for (auto& item : _actionToButtonMap)
	{
		if (!item.second.handledThisFrame)
		{
			SetInputButtonState(item.second, item.second.currentState);
		}
	}
}

bool MouseController::GetButtonDown(int actionId) const
{
	auto inputButton = _actionToButtonMap.find(actionId);
	if (inputButton == _actionToButtonMap.cend())
	{
		return false;
	}

	return (inputButton->second.currentState == ButtonState::Pressed && inputButton->second.previousState != ButtonState::Pressed);
}

bool MouseController::GetButtonPressed(int actionId) const
{
	auto inputButton = _actionToButtonMap.find(actionId);
	if (inputButton == _actionToButtonMap.cend())
	{
		return false;
	}

	return inputButton->second.currentState == ButtonState::Pressed;
}

bool MouseController::GetButtonUp(int actionId) const
{
	auto inputButton = _actionToButtonMap.find(actionId);
	if (inputButton == _actionToButtonMap.cend())
	{
		return false;
	}

	return inputButton->second.currentState == ButtonState::Released;
}

void MouseController::GetPosition(int& x, int& y) const
{
	SDL_GetMouseState(&x, &y);
}

void MouseController::HandleButton(const SDL_Event& event)
{
	uint8_t code = event.button.button;
	auto action = _keyToButtonActionMap.find(code);
	if (action == _keyToButtonActionMap.cend())
	{
		return;
	}

	if (event.button.state == SDL_PRESSED)
	{
		_actionToButtonMap[action->second].handledThisFrame = true;
		SetInputButtonState(_actionToButtonMap[action->second], ButtonState::Pressed);
	}
	else if (event.button.state == SDL_RELEASED)
	{
		_actionToButtonMap[action->second].handledThisFrame = true;
		SetInputButtonState(_actionToButtonMap[action->second], ButtonState::Released);
	}
}

void MouseController::SetInputButtonState(InputButton& button, ButtonState newState)
{
	button.previousState = button.currentState;
	button.currentState = newState;
}
