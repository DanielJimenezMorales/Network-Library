#pragma once
#include "numeric_types.h"

#include <SDL.h>

class IInputController
{
public:
	virtual void HandleEvent(const SDL_Event& event) = 0;
	virtual void ResetEvents() = 0;
	virtual void UpdateUnhandledButtons() = 0;
	virtual bool GetButtonDown(int32 actionId) const = 0;
	virtual bool GetButtonPressed(int32 actionId) const = 0;
	virtual bool GetButtonUp(int32 actionId) const  = 0;
	virtual float32 GetAxis(int32 actionId) const = 0;
};

