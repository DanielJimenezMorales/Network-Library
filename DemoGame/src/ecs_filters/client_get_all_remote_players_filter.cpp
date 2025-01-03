#include "client_get_all_remote_players_filter.h"

#include "GameEntity.hpp"

#include "ecs/entity_container.h"

#include "components/remote_player_controller_component.h"

ClientGetAllRemotePlayersFilter ClientGetAllRemotePlayersFilter::_instance;

ClientGetAllRemotePlayersFilter::ClientGetAllRemotePlayersFilter()
    : ECS::IFilter()
{
}

const ClientGetAllRemotePlayersFilter* ClientGetAllRemotePlayersFilter::GetInstance()
{
	return &_instance;
}

std::vector< GameEntity > ClientGetAllRemotePlayersFilter::Apply( ECS::EntityContainer& entity_container ) const
{
	return entity_container.GetEntitiesOfType< RemotePlayerControllerComponent >();
}
