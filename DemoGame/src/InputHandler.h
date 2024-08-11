#pragma once
#include <SDL.h>
#include <vector>

#include "Logger.h"

class IInputController;
class ICursor;

enum class ButtonState
{
	None,
	Pressed,
	Released
};

struct InputButton
{
	int actionId;
	int code;
	ButtonState currentState;
	ButtonState previousState;
	bool handledThisFrame;

	InputButton() : actionId(0), code(0), currentState(ButtonState::None), previousState(ButtonState::None), handledThisFrame(false) {}
	InputButton(int actionId, int code) : actionId(actionId), code(code), currentState(ButtonState::None), previousState(ButtonState::None), handledThisFrame(false) {}
};

struct InputAxis
{
	int actionId;
	int positiveCode;
	int negativeCode;
	ButtonState positiveState;
	ButtonState negativeState;
	float value;

	InputAxis() : actionId(0), positiveCode(0), negativeCode(0), value(0), positiveState(ButtonState::None), negativeState(ButtonState::None) {}
	InputAxis(int actionId, int positiveCode, int negativeCode) : actionId(actionId), positiveCode(positiveCode), negativeCode(negativeCode), value(0), positiveState(ButtonState::None), negativeState(ButtonState::None) {}
};

class InputHandler
{
public:
	void AddController(IInputController* inputController);
	void AddCursor(ICursor* cursor);
	void PreHandleEvents();
	void HandleEvent(const SDL_Event& event);
	void PostHandleEvents();

private:
	std::vector<IInputController*> _controllers;
	std::vector<ICursor*> _cursors;
};
