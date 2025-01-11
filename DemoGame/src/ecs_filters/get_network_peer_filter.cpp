#include "get_network_peer_filter.h"

#include "GameEntity.hpp"

#include "global_components/network_peer_global_component.h"

#include "ecs/entity_container.h"

GetNetworkPeerFilter GetNetworkPeerFilter::_instance;

GetNetworkPeerFilter::GetNetworkPeerFilter()
    : ECS::IFilter()
{
}

const GetNetworkPeerFilter* GetNetworkPeerFilter::GetInstance()
{
	return &_instance;
}

std::vector< GameEntity > GetNetworkPeerFilter::Apply( ECS::EntityContainer& entity_container ) const
{
	return std::vector< GameEntity >( { entity_container.GetFirstEntityOfType< NetworkPeerGlobalComponent >() } );
}
