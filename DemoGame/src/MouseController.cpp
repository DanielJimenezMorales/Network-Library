#include "MouseController.h"

void MouseController::AddButtonMap(const InputButton& inputButton)
{
	_actionToButtonMap[inputButton.actionId] = inputButton;
	_keyToButtonActionMap[static_cast<uint8>(inputButton.code)] = inputButton.actionId;
}

void MouseController::HandleEvent(const SDL_Event& event)
{
	if (event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEBUTTONUP && event.type != SDL_MOUSEMOTION)
	{
		return;
	}

	if (event.type == SDL_MOUSEMOTION)
	{
		_mouseDeltaX = event.motion.xrel;
		_mouseDeltaY = event.motion.yrel;
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

	_mouseDeltaX = 0;
	_mouseDeltaY = 0;
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

bool MouseController::GetButtonDown(int32 actionId) const
{
	auto inputButton = _actionToButtonMap.find(actionId);
	if (inputButton == _actionToButtonMap.cend())
	{
		return false;
	}

	return (inputButton->second.currentState == ButtonState::Pressed && inputButton->second.previousState != ButtonState::Pressed);
}

bool MouseController::GetButtonPressed(int32 actionId) const
{
	auto inputButton = _actionToButtonMap.find(actionId);
	if (inputButton == _actionToButtonMap.cend())
	{
		return false;
	}

	return inputButton->second.currentState == ButtonState::Pressed;
}

bool MouseController::GetButtonUp(int32 actionId) const
{
	auto inputButton = _actionToButtonMap.find(actionId);
	if (inputButton == _actionToButtonMap.cend())
	{
		return false;
	}

	return inputButton->second.currentState == ButtonState::Released;
}

void MouseController::GetPosition(int32& x, int32& y) const
{
	SDL_GetMouseState(&x, &y);
}

void MouseController::GetDelta(int32& x, int32& y) const
{
	x = _mouseDeltaX;
	y = _mouseDeltaY;
}

void MouseController::HandleButton(const SDL_Event& event)
{
	uint8 code = event.button.button;
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
