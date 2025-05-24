#include "keyboard_controller.h"

namespace Engine
{
	void KeyboardController::AddButtonMap( const InputButton& inputButton )
	{
		_actionToButtonMap[ inputButton.actionId ] = inputButton;
		_keyToButtonActionMap[ inputButton.code ] = inputButton.actionId;
	}

	void KeyboardController::AddAxisMap( const InputAxis& inputAxis )
	{
		_actionToAxisMap[ inputAxis.actionId ] = inputAxis;
		_keyToAxisActionMap[ inputAxis.positiveCode ] = inputAxis.actionId;
		_keyToAxisActionMap[ inputAxis.negativeCode ] = inputAxis.actionId;
	}

	void KeyboardController::HandleEvent( const SDL_Event& event )
	{
		if ( event.type != SDL_KEYDOWN && event.type != SDL_KEYUP )
		{
			return;
		}

		HandleButton( event );
		HandleAxis( event );
	}

	void KeyboardController::ResetEvents()
	{
		for ( auto& item : _actionToButtonMap )
		{
			if ( item.second.currentState == ButtonState::Released )
			{
				SetInputButtonState( item.second, ButtonState::None );
			}

			item.second.handledThisFrame = false;
		}
	}

	void KeyboardController::UpdateUnhandledButtons()
	{
		for ( auto& item : _actionToButtonMap )
		{
			if ( !item.second.handledThisFrame )
			{
				SetInputButtonState( item.second, item.second.currentState );
			}
		}
	}

	bool KeyboardController::GetButtonDown( int32 actionId ) const
	{
		auto inputButton = _actionToButtonMap.find( actionId );
		if ( inputButton == _actionToButtonMap.cend() )
		{
			return false;
		}

		return ( inputButton->second.currentState == ButtonState::Pressed &&
		         inputButton->second.previousState != ButtonState::Pressed );
	}

	bool KeyboardController::GetButtonPressed( int32 actionId ) const
	{
		auto inputButton = _actionToButtonMap.find( actionId );
		if ( inputButton == _actionToButtonMap.cend() )
		{
			return false;
		}

		return inputButton->second.currentState == ButtonState::Pressed;
	}

	bool KeyboardController::GetButtonUp( int32 actionId ) const
	{
		auto inputButton = _actionToButtonMap.find( actionId );
		if ( inputButton == _actionToButtonMap.cend() )
		{
			return false;
		}

		return inputButton->second.currentState == ButtonState::Released;
	}

	float32 KeyboardController::GetAxis( int32 actionId ) const
	{
		auto axis = _actionToAxisMap.find( actionId );
		if ( axis == _actionToAxisMap.cend() )
		{
			return 0.f;
		}

		return axis->second.value;
	}

	void KeyboardController::HandleButton( const SDL_Event& event )
	{
		int32 code = event.key.keysym.sym;
		auto action = _keyToButtonActionMap.find( code );
		if ( action == _keyToButtonActionMap.cend() )
		{
			return;
		}

		if ( event.key.state == SDL_PRESSED )
		{
			_actionToButtonMap[ action->second ].handledThisFrame = true;
			SetInputButtonState( _actionToButtonMap[ action->second ], ButtonState::Pressed );
		}
		else if ( event.key.state == SDL_RELEASED )
		{
			_actionToButtonMap[ action->second ].handledThisFrame = true;
			SetInputButtonState( _actionToButtonMap[ action->second ], ButtonState::Released );
		}
	}

	void KeyboardController::HandleAxis( const SDL_Event& event )
	{
		int32 code = event.key.keysym.sym;
		auto action = _keyToAxisActionMap.find( code );
		if ( action == _keyToAxisActionMap.cend() )
		{
			return;
		}

		if ( event.key.state == SDL_PRESSED )
		{
			if ( code == _actionToAxisMap[ action->second ].negativeCode )
			{
				_actionToAxisMap[ action->second ].negativeState = ButtonState::Pressed;
			}
			else if ( code == _actionToAxisMap[ action->second ].positiveCode )
			{
				_actionToAxisMap[ action->second ].positiveState = ButtonState::Pressed;
			}
		}
		else if ( event.key.state == SDL_RELEASED )
		{
			if ( code == _actionToAxisMap[ action->second ].negativeCode )
			{
				_actionToAxisMap[ action->second ].negativeState = ButtonState::Released;
			}
			else if ( code == _actionToAxisMap[ action->second ].positiveCode )
			{
				_actionToAxisMap[ action->second ].positiveState = ButtonState::Released;
			}
		}

		// Calculate axis value
		_actionToAxisMap[ action->second ].value = 0;
		if ( _actionToAxisMap[ action->second ].positiveState == ButtonState::Pressed )
		{
			_actionToAxisMap[ action->second ].value += 1;
		}

		if ( _actionToAxisMap[ action->second ].negativeState == ButtonState::Pressed )
		{
			_actionToAxisMap[ action->second ].value -= 1;
		}
	}

	void KeyboardController::SetInputButtonState( InputButton& button, ButtonState newState )
	{
		button.previousState = button.currentState;
		button.currentState = newState;
	}
} // namespace Engine
