#pragma once
#include <unordered_map>
#include <SDL.h>

#include "IInputController.h"
#include "InputHandler.h"

class KeyboardController : public IInputController
{
public:
	void AddButtonMap(const InputButton& inputButton);
	void AddAxisMap(const InputAxis& inputAxis);
	void HandleEvent(const SDL_Event& event) override;
	void ResetEvents() override;
	void UpdateUnhandledButtons() override;
	bool GetButtonDown(int32 actionId) const override;
	bool GetButtonPressed(int32 actionId) const override;
	bool GetButtonUp(int32 actionId) const override;
	float32 GetAxis(int32 actionId) const override;

private:
	void HandleButton(const SDL_Event& event);
	void HandleAxis(const SDL_Event& event);
	void SetInputButtonState(InputButton& button, ButtonState newState);

	std::unordered_map<int, InputButton> _actionToButtonMap;
	std::unordered_map<SDL_Keycode, int> _keyToButtonActionMap;

	std::unordered_map<int, InputAxis> _actionToAxisMap;
	std::unordered_map<SDL_Keycode, int> _keyToAxisActionMap;
};

