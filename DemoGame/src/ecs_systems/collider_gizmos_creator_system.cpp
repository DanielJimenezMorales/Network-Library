#include "collider_gizmos_creator_system.h"

#include "GameEntity.hpp"
#include "ServiceLocator.h"
#include "TransformComponent.h"
#include "Collider2DComponent.h"
#include "Gizmo.h"
#include "IGizmoQueryStorage.h"
#include <cassert>

ColliderGizmosCreatorSystem::ColliderGizmosCreatorSystem()
    : ECS::ISimpleSystem()
{
}

void ColliderGizmosCreatorSystem::Execute( GameEntity& entity, float32 elapsed_time )
{
	ServiceLocator& serviceLocator = ServiceLocator::GetInstance();
	IGizmoQueryStorage& gizmoQueryStorage = serviceLocator.GetGizmoQueryStorage();

	const Collider2DComponent& collider = entity.GetComponent< Collider2DComponent >();
	TransformComponent& transform = entity.GetComponent< TransformComponent >();
	Gizmo* gizmo = collider.GetGizmo( transform );
	assert( gizmo != nullptr );

	gizmoQueryStorage.AddGizmo( gizmo );
}
