#pragma once
#include "numeric_types.h"

#include "ICursor.h"
#include "input_button.h"

#include <unordered_map>

namespace Engine
{
	class MouseController : public ICursor
	{
		public:
			MouseController()
			    : ICursor()
			    , _mouseDeltaX( 0 )
			    , _mouseDeltaY( 0 )
			{
			}

			void AddButtonMap( const InputButton& inputButton );
			void HandleEvent( const SDL_Event& event ) override;
			void ResetEvents() override;
			void UpdateUnhandledButtons() override;
			bool GetButtonDown( int32 actionId ) const override;
			bool GetButtonPressed( int32 actionId ) const override;
			bool GetButtonUp( int32 actionId ) const override;
			void GetPosition( int32& x, int32& y ) const override;
			void GetDelta( int32& x, int32& y ) const override;

		private:
			void HandleButton( const SDL_Event& event );

			// TODO Refactor this method inside InputButton struct
			void SetInputButtonState( InputButton& button, ButtonState newState );

			std::unordered_map< int, InputButton > _actionToButtonMap;
			std::unordered_map< uint8, int > _keyToButtonActionMap;

			int32 _mouseDeltaX;
			int32 _mouseDeltaY;
	};
} // namespace Engine
