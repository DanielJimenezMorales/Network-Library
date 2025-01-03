#pragma once
#include "ecs/i_filter.h"

class GetNetworkPeerFilter : public ECS::IFilter
{
	public:
		GetNetworkPeerFilter();

		static const GetNetworkPeerFilter* GetInstance();

		std::vector< GameEntity > Apply( ECS::EntityContainer& entity_container ) const override;

	private:
		static GetNetworkPeerFilter _instance;
};
