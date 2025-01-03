#pragma once
#include "ecs/i_filter.h"

class ServerGetAllPlayersFilter : public ECS::IFilter
{
	public:
		ServerGetAllPlayersFilter();

		static const ServerGetAllPlayersFilter* GetInstance();

		std::vector< GameEntity > Apply( ECS::EntityContainer& entity_container ) const override;

	private:
		static ServerGetAllPlayersFilter _instance;
};
