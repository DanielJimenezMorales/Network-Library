#include "component_registry.h"
#include "component_registry.h"

#include "logger.h"

namespace Engine
{
	namespace ECS
	{
		bool ComponentRegistry::TryAddComponent( const std::string& name, GameEntity& entity,
		                                         EntityContainer& entity_container ) const
		{
			bool result = false;
			auto component_found = _components.find( name );
			if ( component_found != _components.end() )
			{
				component_found->second( entity, entity_container );
				result = true;
			}

			return result;
		}

		void ComponentRegistry::LogComponentAlreadyRegisteredError( const std::string& name )
		{
			LOG_ERROR( "[ECS::ComponentRegistry::LogComponentAlreadyRegisteredError] Component with name %s is already "
			           "registered.",
			           name.c_str() );
		}
	}
}