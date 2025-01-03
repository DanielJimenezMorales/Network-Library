#include "get_all_gizmo_renderer_and_transform_filter.h"

#include "GameEntity.hpp"

#include "ecs/entity_container.h"

#include "components/gizmo_renderer_component.h"
#include "components/transform_component.h"

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
