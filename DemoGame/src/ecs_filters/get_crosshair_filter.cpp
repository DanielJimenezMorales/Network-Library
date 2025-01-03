#include "get_crosshair_filter.h"

#include "GameEntity.hpp"

#include "ecs/entity_container.h"

#include "components/crosshair_component.h"

GetCrosshairFilter GetCrosshairFilter::_instance;

GetCrosshairFilter::GetCrosshairFilter()
    : ECS::IFilter()
{
}

const GetCrosshairFilter* GetCrosshairFilter::GetInstance()
{
	return &_instance;
}

std::vector< GameEntity > GetCrosshairFilter::Apply( ECS::EntityContainer& entity_container ) const
{
	return std::vector< GameEntity >( { entity_container.GetFirstEntityOfType< CrosshairComponent >() } );
}
