#include "GizmoRendererSystem.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include "CameraComponent.h"
#include "ServiceLocator.h"
#include "Gizmo.h"
#include "IGizmoQueryStorage.h"
#include <vector>

void GizmoRendererSystem::Render(EntityContainer& entityContainer, SDL_Renderer* renderer) const
{
	const GameEntity cameraEntity = entityContainer.GetFirstEntityOfType<CameraComponent>();
	const CameraComponent& cameraComponent = cameraEntity.GetComponent<CameraComponent>();

	ServiceLocator& serviceLocator = ServiceLocator::GetInstance();
	IGizmoQueryStorage& gizmoQueryStorage = serviceLocator.GetGizmoQueryStorage();
	const std::vector<Gizmo*>& gizmos = gizmoQueryStorage.GetGizmos();
	auto gizmos_cit = gizmos.cbegin();
	for (; gizmos_cit != gizmos.cend(); ++gizmos_cit)
	{
		(*gizmos_cit)->Render(cameraComponent, renderer);
	}

	gizmoQueryStorage.ClearGizmos();

	//TODO cache the color in a variable so I dont need to hardcode it in different places of the code
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
}
