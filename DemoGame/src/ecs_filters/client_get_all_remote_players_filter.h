#pragma once
#include "ecs/i_filter.h"

class ClientGetAllRemotePlayersFilter : public ECS::IFilter
{
	public:
		ClientGetAllRemotePlayersFilter();

		static const ClientGetAllRemotePlayersFilter* GetInstance();

		std::vector< GameEntity > Apply( ECS::EntityContainer& entity_container ) const override;

	private:
		static ClientGetAllRemotePlayersFilter _instance;
};
