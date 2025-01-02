#include "server_get_all_players_filter.h"

#include "ecs/entity_container.h"

#include "PlayerControllerComponent.h"
#include "GameEntity.hpp"

ServerGetAllPlayersFilter ServerGetAllPlayersFilter::_instance;

ServerGetAllPlayersFilter::ServerGetAllPlayersFilter()
    : IFilter()
{
}

const ServerGetAllPlayersFilter* ServerGetAllPlayersFilter::GetInstance()
{
	return &_instance;
}

std::vector< GameEntity > ServerGetAllPlayersFilter::Apply( ECS::EntityContainer& entity_container ) const
{
	return entity_container.GetEntitiesOfType< PlayerControllerComponent >();
}
