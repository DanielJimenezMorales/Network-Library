#include "get_all_virtual_mouse_filter.h"

#include "ecs/entity_container.h"

#include "GameEntity.hpp"

#include "VirtualMouseComponent.h"

GetAllVirtualMouseFilter GetAllVirtualMouseFilter::_instance;

GetAllVirtualMouseFilter::GetAllVirtualMouseFilter()
    : ECS::IFilter()
{
}

const GetAllVirtualMouseFilter* GetAllVirtualMouseFilter::GetInstance()
{
	return &_instance;
}

std::vector< GameEntity > GetAllVirtualMouseFilter::Apply( ECS::EntityContainer& entity_container ) const
{
	return entity_container.GetEntitiesOfType< VirtualMouseComponent >();
}
