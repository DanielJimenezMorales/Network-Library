#pragma once
#include <SDL.h>

class IInputController
{
public:
	virtual void HandleEvent(const SDL_Event& event) = 0;
	virtual void ResetEvents() = 0;
	virtual void UpdateUnhandledButtons() = 0;
	virtual bool GetButtonDown(int actionId) const = 0;
	virtual bool GetButtonPressed(int actionId) const = 0;
	virtual bool GetButtonUp(int actionId) const  = 0;
	virtual float GetAxis(int actionId) const = 0;
};

