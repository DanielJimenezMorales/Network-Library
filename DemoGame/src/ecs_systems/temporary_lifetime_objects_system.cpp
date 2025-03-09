#include "temporary_lifetime_objects_system.h"

#include "ecs/entity_container.h"
#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/temporary_lifetime_component.h"

#include "component_configurations/temporary_lifetime_component_configuration.h"

#include <vector>

TemporaryLifetimeObjectsSystem::TemporaryLifetimeObjectsSystem( ECS::World* world )
    : ECS::ISimpleSystem()
    , _world( world )
{
}

void TemporaryLifetimeObjectsSystem::Execute( ECS::EntityContainer& entity_container, float32 elapsed_time )
{
	std::vector< ECS::GameEntity > temporary_lifetime_entities =
	    entity_container.GetEntitiesOfType< TemporaryLifetimeComponent >();
	for ( auto it = temporary_lifetime_entities.begin(); it != temporary_lifetime_entities.end(); ++it )
	{
		TemporaryLifetimeComponent& temporary_lifetime = it->GetComponent< TemporaryLifetimeComponent >();
		temporary_lifetime.lifetimeLeft -= elapsed_time;

		if ( temporary_lifetime.lifetimeLeft <= 0.f )
		{
			_world->DestroyGameEntity( *it );
		}
	}
}

void TemporaryLifetimeObjectsSystem::ConfigureTemporaryLifetimeComponent( ECS::GameEntity& entity,
                                                                          const ECS::Prefab& prefab )
{
	auto component_config_found = prefab.componentConfigurations.find( "TemporaryLifetime" );
	if ( component_config_found == prefab.componentConfigurations.end() )
	{
		return;
	}

	if ( !entity.HasComponent< TemporaryLifetimeComponent >() )
	{
		return;
	}

	const TemporaryLifetimeComponentConfiguration& component_config =
	    static_cast< const TemporaryLifetimeComponentConfiguration& >( *component_config_found->second );
	TemporaryLifetimeComponent& temporary_lifetime = entity.GetComponent< TemporaryLifetimeComponent >();

	temporary_lifetime.lifetime = component_config.lifetime;
	temporary_lifetime.lifetimeLeft = component_config.lifetime;
}
