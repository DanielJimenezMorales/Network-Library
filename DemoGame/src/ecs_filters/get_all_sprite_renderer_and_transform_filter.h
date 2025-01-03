#pragma once
#include "ecs/i_filter.h"

class GetAllSpriteRendererAndTransformFilter : public ECS::IFilter
{
	public:
		GetAllSpriteRendererAndTransformFilter();

		static const GetAllSpriteRendererAndTransformFilter* GetInstance();

		std::vector< GameEntity > Apply( ECS::EntityContainer& entity_container ) const override;

	private:
		static GetAllSpriteRendererAndTransformFilter _instance;
};
