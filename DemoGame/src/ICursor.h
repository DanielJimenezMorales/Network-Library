#pragma once
#include <SDL.h>

class ICursor
{
public:
	virtual void HandleEvent(const SDL_Event& event) = 0;
	virtual void ResetEvents() = 0;
	virtual void UpdateUnhandledButtons() = 0;
	virtual bool GetButtonDown(int actionId) const = 0;
	virtual bool GetButtonPressed(int actionId) const = 0;
	virtual bool GetButtonUp(int actionId) const = 0;
	virtual void GetPosition(int& x, int& y) const = 0;
};