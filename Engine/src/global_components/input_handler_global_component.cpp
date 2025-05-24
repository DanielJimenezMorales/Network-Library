#include "input_handler_global_component.h"

#include "inputs/IInputController.h"
#include "inputs/ICursor.h"

#include "logger.h"

namespace Engine
{
	bool InputHandlerGlobalComponent::ControllerGetButtonDown( const std::string& name, int32 action_id ) const
	{
		bool result = false;

		const auto controller_found = controllers.find( name );
		if ( controller_found != controllers.cend() )
		{
			result = controller_found->second->GetButtonDown( action_id );
		}
		else
		{
			LOG_WARNING( "[InputHandlerGlobalComponent::ControllerGetButtonDown] Controller with name '%s' not found.",
			             name.c_str() );
		}

		return result;
	}

	bool InputHandlerGlobalComponent::ControllerGetButtonPressed( const std::string& name, int32 action_id ) const
	{
		bool result = false;

		const auto controller_found = controllers.find( name );
		if ( controller_found != controllers.cend() )
		{
			result = controller_found->second->GetButtonPressed( action_id );
		}
		else
		{
			LOG_WARNING(
			    "[InputHandlerGlobalComponent::ControllerGetButtonPressed] Controller with name '%s' not found.",
			    name.c_str() );
		}

		return result;
	}

	bool InputHandlerGlobalComponent::ControllerGetButtonUp( const std::string& name, int32 action_id ) const
	{
		bool result = false;

		const auto controller_found = controllers.find( name );
		if ( controller_found != controllers.cend() )
		{
			result = controller_found->second->GetButtonUp( action_id );
		}
		else
		{
			LOG_WARNING( "[InputHandlerGlobalComponent::ControllerGetButtonUp] Controller with name '%s' not found.",
			             name.c_str() );
		}

		return result;
	}

	float32 InputHandlerGlobalComponent::ControllerGetAxis( const std::string& name, int32 action_id ) const
	{
		float32 result = 0.f;

		const auto controller_found = controllers.find( name );
		if ( controller_found != controllers.cend() )
		{
			result = controller_found->second->GetAxis( action_id );
		}
		else
		{
			LOG_WARNING( "[InputHandlerGlobalComponent::ControllerGetAxis] Controller with name '%s' not found.",
			             name.c_str() );
		}

		return result;
	}

	bool InputHandlerGlobalComponent::CursorGetButtonDown( const std::string& name, int32 action_id ) const
	{
		bool result = false;

		const auto cursor_found = cursors.find( name );
		if ( cursor_found != cursors.cend() )
		{
			result = cursor_found->second->GetButtonDown( action_id );
		}
		else
		{
			LOG_WARNING( "[InputHandlerGlobalComponent::CursorGetButtonDown] Cursor with name '%s' not found.",
			             name.c_str() );
		}

		return result;
	}

	bool InputHandlerGlobalComponent::CursorGetButtonPressed( const std::string& name, int32 action_id ) const
	{
		bool result = false;

		const auto cursor_found = cursors.find( name );
		if ( cursor_found != cursors.cend() )
		{
			result = cursor_found->second->GetButtonPressed( action_id );
		}
		else
		{
			LOG_WARNING( "[InputHandlerGlobalComponent::CursorGetButtonPressed] Cursor with name '%s' not found.",
			             name.c_str() );
		}

		return result;
	}

	bool InputHandlerGlobalComponent::CursorGetButtonUp( const std::string& name, int32 action_id ) const
	{
		bool result = false;

		const auto cursor_found = cursors.find( name );
		if ( cursor_found != cursors.cend() )
		{
			result = cursor_found->second->GetButtonUp( action_id );
		}
		else
		{
			LOG_WARNING( "[InputHandlerGlobalComponent::CursorGetButtonUp] Cursor with name '%s' not found.",
			             name.c_str() );
		}

		return result;
	}

	void InputHandlerGlobalComponent::CursorGetPosition( const std::string& name, int32& x, int32& y ) const
	{
		x = 0;
		y = 0;

		const auto cursor_found = cursors.find( name );
		if ( cursor_found != cursors.cend() )
		{
			cursor_found->second->GetDelta( x, y );
		}
		else
		{
			LOG_WARNING( "[InputHandlerGlobalComponent::CursorGetPosition] Cursor with name '%s' not found.",
			             name.c_str() );
		}
	}

	void InputHandlerGlobalComponent::CursorGetDelta( const std::string& name, int32& x, int32& y ) const
	{
		x = 0;
		y = 0;

		const auto cursor_found = cursors.find( name );
		if ( cursor_found != cursors.cend() )
		{
			cursor_found->second->GetDelta( x, y );
		}
		else
		{
			LOG_WARNING( "[InputHandlerGlobalComponent::CursorGetDelta] Cursor with name '%s' not found.",
			             name.c_str() );
		}
	}
} // namespace Engine
