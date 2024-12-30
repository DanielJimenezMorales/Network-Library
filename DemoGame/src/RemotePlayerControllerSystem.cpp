#include "RemotePlayerControllerSystem.h"
#include "GameEntity.hpp"
#include "TransformComponent.h"
#include "RemotePlayerControllerComponent.h"
#include <vector>

void RemotePlayerControllerSystem::Tick(EntityContainer& entityContainer, float32 elapsedTime) const
{
	std::vector<GameEntity> remotePlayerEntities = entityContainer.GetEntitiesOfType< RemotePlayerControllerComponent>();
	auto it = remotePlayerEntities.begin();
	for (; it != remotePlayerEntities.end(); ++it)
	{
		TransformComponent& transform = it->GetComponent<TransformComponent>();
		const RemotePlayerControllerComponent& networkComponent = it->GetComponent<RemotePlayerControllerComponent>();

		//transform.SetPosition(Vec2f(networkComponent.posX.Get(), networkComponent.posY.Get()));
		//transform.SetRotationAngle(networkComponent.rotationAngle.Get());
	}
}
