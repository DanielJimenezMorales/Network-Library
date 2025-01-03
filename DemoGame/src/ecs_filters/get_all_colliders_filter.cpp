#include "get_all_colliders_filter.h"

#include "GameEntity.hpp"

#include "ecs/entity_container.h"

#include "components/transform_component.h"
#include "components/collider_2d_component.h"

GetAllCollidersFilter GetAllCollidersFilter::_instance;

GetAllCollidersFilter::GetAllCollidersFilter()
    : ECS::IFilter()
{
}

const GetAllCollidersFilter* GetAllCollidersFilter::GetInstance()
{
	return &_instance;
}

std::vector< GameEntity > GetAllCollidersFilter::Apply( ECS::EntityContainer& entity_container ) const
{
	return entity_container.GetEntitiesOfBothTypes< Collider2DComponent, TransformComponent >();
}
