#include "pos_tick_network_system.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "global_components/network_peer_global_component.h"

PosTickNetworkSystem::PosTickNetworkSystem()
    : Engine::ECS::ISimpleSystem()
{
}

void PosTickNetworkSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	networkPeerComponent.peer->Tick( elapsed_time );
}
