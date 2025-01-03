#include "get_all_virtual_mouse_filter.h"

#include "GameEntity.hpp"

#include "ecs/entity_container.h"

#include "components/virtual_mouse_component.h"

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
