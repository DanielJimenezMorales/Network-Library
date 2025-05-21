#pragma once
#include <string>
#include <functional>
#include <unordered_map>

#include "ecs/entity_container.h"

class GameEntity;

namespace Engine
{
	namespace ECS
	{
		class ComponentRegistry
		{
			public:
				template < typename T >
				bool RegisterComponent( const std::string& name );

				bool TryAddComponent( const std::string& name, GameEntity& entity,
				                      ECS::EntityContainer& entity_container ) const;

			private:
				using CreatorFunction = std::function< void( GameEntity&, ECS::EntityContainer& ) >;

				void LogComponentAlreadyRegisteredError( const std::string& name );

				std::unordered_map< std::string, CreatorFunction > _components;
		};

		template < typename T >
		inline bool ComponentRegistry::RegisterComponent( const std::string& name )
		{
			bool result = false;
			auto component_found = _components.find( name );
			if ( component_found == _components.end() )
			{
				_components[ name ] = []( GameEntity& entity, ECS::EntityContainer& entity_container )
				{
					entity_container.AddComponentToEntity< T >( entity );
				};

				result = true;
			}
			else
			{
				LogComponentAlreadyRegisteredError( name );
			}

			return result;
		}
	} // namespace ECS
} // namespace Engine
