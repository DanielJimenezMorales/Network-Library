#include "server_remove_death_entities_system.h"
#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "shared/components/health_component.h"
#include "shared/global_components/network_peer_global_component.h"
#include "shared/components/network_entity_component.h"

#include "server/global_components/server_dead_players_to_revive_global_component.h"

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
				NetLib::Server* server = networkPeerComponent.GetPeerAsServer();
				server->DestroyNetworkEntity( networkEntityComponent.networkEntityId );
				// TODO Move this to a player factory
				server->DisableInputBufferForRemotePeer( networkEntityComponent.controlledByPeerId );
				LOG_INFO( "Removing networked player entity." );

				// Add player entity to dead entities to revive in order to create a new one soon.
				// TODO This should not be hardcoded here. In general, the OnNetworkEntityCreate and destroy code is
				// pretty spaguetti and needs to be fixed to add some flexibility and be easier to maintain
				ServerDeadPlayersToReviveGlobalComponent& deadPlayersToReviveComponent =
				    world.GetGlobalComponent< ServerDeadPlayersToReviveGlobalComponent >();
				DeadPlayerEntry deadPlayerEntry;
				deadPlayerEntry.remotePeerId = networkEntityComponent.controlledByPeerId;
				deadPlayerEntry.timeLeft = 2.f;
				deadPlayersToReviveComponent.deadPlayersToRevive.push_back( deadPlayerEntry );
			}
			else
			{
				world.DestroyGameEntity( *cit );
			}
		}
	}
}
