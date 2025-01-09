#include "pos_tick_network_system.h"

#include "GameEntity.hpp"

#include "components/network_peer_component.h"

PosTickNetworkSystem::PosTickNetworkSystem()
    : ECS::ISimpleSystem()
{
}

void PosTickNetworkSystem::Execute( std::vector< GameEntity >& entities, ECS::EntityContainer& entity_container,
                                    float32 elapsed_time )
{
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		NetworkPeerComponent& networkPeerComponent = it->GetComponent< NetworkPeerComponent >();
		networkPeerComponent.peer->Tick( elapsed_time );
	}
}
