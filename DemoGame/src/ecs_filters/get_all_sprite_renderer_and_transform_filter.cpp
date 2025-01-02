#include "get_all_sprite_renderer_and_transform_filter.h"

#include "ecs/entity_container.h"

#include "SpriteRendererComponent.h"
#include "TransformComponent.h"
#include "GameEntity.hpp"

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
