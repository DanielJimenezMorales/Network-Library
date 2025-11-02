#include "server_revive_dead_players_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "server/global_components/server_dead_players_to_revive_global_component.h"

#include "shared/global_components/network_peer_global_component.h"
#include "shared/networked_entity_types.h"

void ServerReviveDeadPlayersSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	ServerDeadPlayersToReviveGlobalComponent& deadPlayersToReviveComponent =
	    world.GetGlobalComponent< ServerDeadPlayersToReviveGlobalComponent >();

	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();

	// Update revival time left for each dead player
	auto it = deadPlayersToReviveComponent.deadPlayersToRevive.begin();
	for ( ; it != deadPlayersToReviveComponent.deadPlayersToRevive.end(); ++it )
	{
		it->timeLeft -= elapsed_time;
	}

	// Revive dead players whose time left is up
	for ( int32 i = deadPlayersToReviveComponent.deadPlayersToRevive.size() - 1; i >= 0; --i )
	{
		if ( deadPlayersToReviveComponent.deadPlayersToRevive[ i ].timeLeft <= 0.f )
		{
			const uint32 remotePeerId = deadPlayersToReviveComponent.deadPlayersToRevive[ i ].remotePeerId;

			deadPlayersToReviveComponent.deadPlayersToRevive.erase(
			    deadPlayersToReviveComponent.deadPlayersToRevive.cbegin() + i );

			NetLib::Server* server = networkPeerComponent.GetPeerAsServer();
			const NetLib::RemotePeerState remotePeerState = server->GetRemotePeerState( remotePeerId );
			if ( remotePeerState == NetLib::RemotePeerState::Connected )
			{
				server->CreateNetworkEntity( PLAYER_NETWORKED_ENTITY_TYPE, remotePeerId, 0.f, 0.f );
				// TODO Move this to a network player factory
				server->EnableInputBufferForRemotePeer( remotePeerId );
			}
			else
			{
				LOG_INFO( "ServerReviveDeadPlayersSystem::%s, Trying to revive a player from a remote peer id that is "
				          "not connected. Probably the remote peer disconnected while the player was waiting for being "
				          "revived.",
				          THIS_FUNCTION_NAME );
			}
		}
	}
}
