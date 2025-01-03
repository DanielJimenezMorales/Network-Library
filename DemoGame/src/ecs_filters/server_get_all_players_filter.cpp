#include "server_get_all_players_filter.h"

#include "GameEntity.hpp"

#include "ecs/entity_container.h"

#include "components/player_controller_component.h"

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
