#pragma once
#include "ecs/i_filter.h"

class GetAllGizmoRendererAndTransformFilter : public ECS::IFilter
{
	public:
		GetAllGizmoRendererAndTransformFilter();

		static const GetAllGizmoRendererAndTransformFilter* GetInstance();

		std::vector< GameEntity > Apply( ECS::EntityContainer& entity_container ) const override;

	private:
		static GetAllGizmoRendererAndTransformFilter _instance;
};
