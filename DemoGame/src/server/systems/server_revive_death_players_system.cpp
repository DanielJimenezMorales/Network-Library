#include "server_revive_dead_players_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "server/global_components/server_dead_players_to_revive_global_component.h"

#include "shared/global_components/network_peer_global_component.h"

void ServerReviveDeadPlayersSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	ServerDeadPlayersToReviveGlobalComponent& deadPlayersToReviveComponent =
	    world.GetGlobalComponent< ServerDeadPlayersToReviveGlobalComponent >();

	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();

	auto it = deadPlayersToReviveComponent.deadPlayersToRevive.begin();
	for ( ; it != deadPlayersToReviveComponent.deadPlayersToRevive.end(); ++it )
	{
		it->timeLeft -= elapsed_time;
	}

	for ( int32 i = deadPlayersToReviveComponent.deadPlayersToRevive.size() - 1; i >= 0; --i )
	{
		if ( deadPlayersToReviveComponent.deadPlayersToRevive[ i ].timeLeft <= 0.f )
		{
			const uint32 remotePeerId = deadPlayersToReviveComponent.deadPlayersToRevive[ i ].remotePeerId;

			deadPlayersToReviveComponent.deadPlayersToRevive.erase(
			    deadPlayersToReviveComponent.deadPlayersToRevive.cbegin() + i );

			networkPeerComponent.GetPeerAsServer()->CreateNetworkEntity( 10, remotePeerId, 0.f, 0.f );
		}
	}
}
