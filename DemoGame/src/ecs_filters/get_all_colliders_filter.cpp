#include "get_all_colliders_filter.h"

#include "ecs/entity_container.h"

#include "TransformComponent.h"
#include "Collider2DComponent.h"
#include "GameEntity.hpp"

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
