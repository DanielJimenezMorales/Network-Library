#pragma once
#include "ecs/i_filter.h"

class GetAllCollidersFilter : public ECS::IFilter
{
	public:
		GetAllCollidersFilter();

		static const GetAllCollidersFilter* GetInstance();

		std::vector< GameEntity > Apply( ECS::EntityContainer& entity_container ) const override;

	private:
		static GetAllCollidersFilter _instance;
};
