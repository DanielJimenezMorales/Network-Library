#include "pos_tick_network_system.h"

#include "GameEntity.hpp"

#include "components/network_peer_component.h"

PosTickNetworkSystem::PosTickNetworkSystem()
    : ECS::ISimpleSystem()
{
}

void PosTickNetworkSystem::Execute( GameEntity& entity, float32 elapsed_time )
{
	NetworkPeerComponent& networkPeerComponent = entity.GetComponent< NetworkPeerComponent >();
	networkPeerComponent.peer->Tick( elapsed_time );
}
