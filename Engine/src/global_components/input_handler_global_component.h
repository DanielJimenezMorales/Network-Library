#pragma once
#include <string>
#include <unordered_map>

#include "numeric_types.h"
#include "Vec2f.h"

namespace Engine
{
	class IInputController;
	class ICursor;

	struct InputHandlerGlobalComponent
	{
			bool ControllerGetButtonDown( const std::string& name, int32 action_id ) const;
			bool ControllerGetButtonPressed( const std::string& name, int32 action_id ) const;
			bool ControllerGetButtonUp( const std::string& name, int32 action_id ) const;
			float32 ControllerGetAxis( const std::string& name, int32 action_id ) const;

			bool CursorGetButtonDown( const std::string& name, int32 action_id ) const;
			bool CursorGetButtonPressed( const std::string& name, int32 action_id ) const;
			bool CursorGetButtonUp( const std::string& name, int32 action_id ) const;
			void CursorGetPosition( const std::string& name, int32& x, int32& y ) const;
			void CursorGetDelta( const std::string& name, int32& x, int32& y ) const;

			std::unordered_map< std::string, IInputController* > controllers;
			std::unordered_map< std::string, ICursor* > cursors;
	};
}
