#pragma once
#include "ecs/i_filter.h"

class GetCrosshairFilter : public ECS::IFilter
{
	public:
		GetCrosshairFilter();

		static const GetCrosshairFilter* GetInstance();

		std::vector< GameEntity > Apply( ECS::EntityContainer& entity_container ) const override;

	private:
		static GetCrosshairFilter _instance;
};
