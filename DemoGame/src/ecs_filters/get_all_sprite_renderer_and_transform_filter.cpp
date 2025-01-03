#include "get_all_sprite_renderer_and_transform_filter.h"

#include "GameEntity.hpp"

#include "ecs/entity_container.h"

#include "components/sprite_renderer_component.h"
#include "components/transform_component.h"

GetAllSpriteRendererAndTransformFilter GetAllSpriteRendererAndTransformFilter::_instance;

GetAllSpriteRendererAndTransformFilter::GetAllSpriteRendererAndTransformFilter()
    : ECS::IFilter()
{
}

const GetAllSpriteRendererAndTransformFilter* GetAllSpriteRendererAndTransformFilter::GetInstance()
{
	return &_instance;
}

std::vector< GameEntity > GetAllSpriteRendererAndTransformFilter::Apply( ECS::EntityContainer& entity_container ) const
{
	return entity_container.GetEntitiesOfBothTypes< SpriteRendererComponent, TransformComponent >();
}
