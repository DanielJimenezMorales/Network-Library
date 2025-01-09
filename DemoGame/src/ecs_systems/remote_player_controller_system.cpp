#include "remote_player_controller_system.h"

#include "GameEntity.hpp"

#include "components/transform_component.h"
#include "components/remote_player_controller_component.h"

RemotePlayerControllerSystem::RemotePlayerControllerSystem()
    : ECS::ISimpleSystem()
{
}

void RemotePlayerControllerSystem::Execute( std::vector< GameEntity >& entities, ECS::EntityContainer& entity_container,
                                            float32 elapsed_time )
{
	// TODO Is this still valid?
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		TransformComponent& transform = it->GetComponent< TransformComponent >();
		const RemotePlayerControllerComponent& networkComponent = it->GetComponent< RemotePlayerControllerComponent >();
	}

	// transform.SetPosition(Vec2f(networkComponent.posX.Get(), networkComponent.posY.Get()));
	// transform.SetRotationAngle(networkComponent.rotationAngle.Get());
}
