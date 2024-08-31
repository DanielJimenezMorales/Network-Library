#include "ColliderGizmosCreatorSystem.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include "ServiceLocator.h"
#include "TransformComponent.h"
#include "Collider2DComponent.h"
#include "Gizmo.h"
#include "IGizmoQueryStorage.h"
#include <cassert>

void ColliderGizmosCreatorSystem::Update(EntityContainer& entityContainer, float32 elapsedTime) const
{
	std::vector<GameEntity> collision_entities = entityContainer.GetEntitiesOfBothTypes < Collider2DComponent, TransformComponent>();

	ServiceLocator& serviceLocator = ServiceLocator::GetInstance();
	IGizmoQueryStorage& gizmoQueryStorage = serviceLocator.GetGizmoQueryStorage();

	for (uint32 i = 0; i < collision_entities.size(); ++i)
	{
		const Collider2DComponent& collider = collision_entities[i].GetComponent<Collider2DComponent>();
		TransformComponent& transform = collision_entities[i].GetComponent<TransformComponent>();
		Gizmo* gizmo = collider.GetGizmo(transform);
		assert(gizmo != nullptr);

		gizmoQueryStorage.AddGizmo(gizmo);
	}
}
