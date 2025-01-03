#include "get_all_gizmo_renderer_and_transform_filter.h"

#include "ecs/entity_container.h"

#include "GameEntity.hpp"

#include "gizmo_renderer_component.h"
#include "TransformComponent.h"

GetAllGizmoRendererAndTransformFilter GetAllGizmoRendererAndTransformFilter::_instance;

GetAllGizmoRendererAndTransformFilter::GetAllGizmoRendererAndTransformFilter()
    : IFilter()
{
}

const GetAllGizmoRendererAndTransformFilter* GetAllGizmoRendererAndTransformFilter::GetInstance()
{
	return &_instance;
}

std::vector< GameEntity > GetAllGizmoRendererAndTransformFilter::Apply( ECS::EntityContainer& entity_container ) const
{
	return entity_container.GetEntitiesOfBothTypes< GizmoRendererComponent, TransformComponent >();
}
