#include "remote_player_controller_system.h"

#include "GameEntity.hpp"

#include "components/transform_component.h"
#include "components/remote_player_controller_component.h"

RemotePlayerControllerSystem::RemotePlayerControllerSystem()
    : ECS::ISimpleSystem()
{
}

void RemotePlayerControllerSystem::Execute( GameEntity& entity, float32 elapsed_time )
{
	TransformComponent& transform = entity.GetComponent< TransformComponent >();
	const RemotePlayerControllerComponent& networkComponent = entity.GetComponent< RemotePlayerControllerComponent >();

	// transform.SetPosition(Vec2f(networkComponent.posX.Get(), networkComponent.posY.Get()));
	// transform.SetRotationAngle(networkComponent.rotationAngle.Get());
}
