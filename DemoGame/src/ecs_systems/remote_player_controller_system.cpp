#include "remote_player_controller_system.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/transform_component.h"
#include "components/remote_player_controller_component.h"

RemotePlayerControllerSystem::RemotePlayerControllerSystem()
    : ECS::ISimpleSystem()
{
}

void RemotePlayerControllerSystem::Execute( ECS::World& world, float32 elapsed_time )
{
	std::vector< ECS::GameEntity > entities = world.GetEntitiesOfType< RemotePlayerControllerComponent >();
	// TODO Is this still valid?
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		TransformComponent& transform = it->GetComponent< TransformComponent >();
		const RemotePlayerControllerComponent& networkComponent = it->GetComponent< RemotePlayerControllerComponent >();
	}

	// transform.SetPosition(Vec2f(networkComponent.posX.Get(), networkComponent.posY.Get()));
	// transform.SetRotationAngle(networkComponent.rotationAngle.Get());
}
