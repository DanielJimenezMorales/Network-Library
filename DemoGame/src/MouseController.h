#pragma once
#include "ICursor.h"
#include "InputHandler.h"
#include <unordered_map>

class MouseController : public ICursor
{
public:
	MouseController() : ICursor(), _mouseDeltaX(0), _mouseDeltaY(0)
	{
	}

	void AddButtonMap(const InputButton& inputButton);
	void HandleEvent(const SDL_Event& event) override;
	void ResetEvents() override;
	void UpdateUnhandledButtons() override;
	bool GetButtonDown(int actionId) const override;
	bool GetButtonPressed(int actionId) const override;
	bool GetButtonUp(int actionId) const override;
	void GetPosition(int& x, int& y) const override;
	void GetDelta(int& x, int& y) const override;

private:
	void HandleButton(const SDL_Event& event);

	//TODO Refactor this method inside InputButton struct
	void SetInputButtonState(InputButton& button, ButtonState newState);

	std::unordered_map<int, InputButton> _actionToButtonMap;
	std::unordered_map<uint8_t, int> _keyToButtonActionMap;

	int _mouseDeltaX;
	int _mouseDeltaY;
};