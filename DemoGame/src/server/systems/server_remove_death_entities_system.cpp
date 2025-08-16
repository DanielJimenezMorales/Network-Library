#include "server_remove_death_entities_system.h"
#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "shared/components/health_component.h"
#include "shared/global_components/network_peer_global_component.h"
#include "shared/components/network_entity_component.h"

void ServerRemoveDeathEntitiesSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();

	const std::vector< Engine::ECS::GameEntity > entitiesWithHealth = world.GetEntitiesOfType< HealthComponent >();
	auto cit = entitiesWithHealth.cbegin();
	for ( ; cit != entitiesWithHealth.cend(); ++cit )
	{
		const HealthComponent& healthComponent = cit->GetComponent< HealthComponent >();
		if ( healthComponent.currentHealth == 0 )
		{
			if ( cit->HasComponent< NetworkEntityComponent >() )
			{
				const NetworkEntityComponent& networkEntityComponent = cit->GetComponent< NetworkEntityComponent >();
				networkPeerComponent.GetPeerAsServer()->DestroyNetworkEntity( networkEntityComponent.networkEntityId );
			}
			else
			{
				world.DestroyGameEntity( *cit );
			}
		}
	}
}
