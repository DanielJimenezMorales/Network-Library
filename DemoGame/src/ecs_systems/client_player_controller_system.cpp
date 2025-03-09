#include "client_player_controller_system.h"

#include "IInputController.h"
#include "ICursor.h"
#include "Vec2f.h"
#include "InputActionIdsConfiguration.h"
#include "InputState.h"
#include "raycaster.h"

#include <vector>

#include "ecs/game_entity.hpp"
#include "ecs/entity_container.h"
#include "ecs/prefab.h"
#include "ecs/world.h"

#include "components/virtual_mouse_component.h"
#include "components/input_component.h"
#include "components/transform_component.h"
#include "components/collider_2d_component.h"
#include "components/player_controller_component.h"

#include "global_components/network_peer_global_component.h"

#include "core/client.h"

ClientPlayerControllerSystem::ClientPlayerControllerSystem( ECS::World* world )
    : ECS::ISimpleSystem()
    , _world( world )
{
}

static void ProcessInputs( ECS::EntityContainer& entityContainer, InputState& outInputState )
{
	const InputComponent& inputComponent = entityContainer.GetGlobalComponent< InputComponent >();

	outInputState.movement.X( inputComponent.inputController->GetAxis( HORIZONTAL_AXIS ) );
	outInputState.movement.Y( inputComponent.inputController->GetAxis( VERTICAL_AXIS ) );
	outInputState.movement.Normalize();

	outInputState.isShooting = inputComponent.cursor->GetButtonPressed( SHOOT_BUTTON );

	const ECS::GameEntity& virtual_mouse_entity = entityContainer.GetFirstEntityOfType< VirtualMouseComponent >();
	const TransformComponent& virtual_mouse_transform = virtual_mouse_entity.GetComponent< TransformComponent >();
	outInputState.virtualMousePosition = virtual_mouse_transform.GetPosition();
}

static void SendInputsToServer( ECS::EntityContainer& entityContainer, const InputState& inputState )
{
	NetworkPeerGlobalComponent& networkPeerComponent =
	    entityContainer.GetGlobalComponent< NetworkPeerGlobalComponent >();

	NetLib::Client& networkClient = *static_cast< NetLib::Client* >( networkPeerComponent.peer );
	networkClient.SendInputs( inputState );
}

void ClientPlayerControllerSystem::Execute( ECS::EntityContainer& entity_container, float32 elapsed_time )
{
	const NetworkPeerGlobalComponent& networkPeerComponent =
	    entity_container.GetGlobalComponent< NetworkPeerGlobalComponent >();
	if ( networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected )
	{
		return;
	}

	InputState inputState;
	ProcessInputs( entity_container, inputState );
	SendInputsToServer( entity_container, inputState );

	if ( inputState.isShooting )
	{
		const ECS::GameEntity local_player = entity_container.GetFirstEntityOfType< PlayerControllerComponent >();
		const TransformComponent& local_player_transform = local_player.GetComponent< TransformComponent >();

		Raycaster::Ray ray;
		ray.origin = local_player_transform.GetPosition();
		ray.direction = local_player_transform.ConvertRotationAngleToNormalizedDirection();
		ray.maxDistance = 100;

		const std::vector< ECS::GameEntity > entities_with_colliders =
		    entity_container.GetEntitiesOfBothTypes< Collider2DComponent, TransformComponent >();
		const Raycaster::RaycastResult result = Raycaster::ExecuteRaycast( ray, entities_with_colliders, local_player );
		if ( result.entity.IsValid() )
		{
			bool r = true;
		}

		ECS::GameEntity entity = _world->CreateGameEntity( "Raycast", ray.origin, ray.direction );
	}
}
