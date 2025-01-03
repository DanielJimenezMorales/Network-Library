#pragma once
#include "ecs/i_filter.h"

class GetAllVirtualMouseFilter : public ECS::IFilter
{
	public:
		GetAllVirtualMouseFilter();

		static const GetAllVirtualMouseFilter* GetInstance();

		std::vector< GameEntity > Apply( ECS::EntityContainer& entity_container ) const override;

	private:
		static GetAllVirtualMouseFilter _instance;
};
