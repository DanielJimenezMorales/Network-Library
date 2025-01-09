#include "client_player_controller_system.h"

#include "GameEntity.hpp"
#include "IInputController.h"
#include "ICursor.h"
#include "Vec2f.h"
#include "InputActionIdsConfiguration.h"
#include "InputState.h"

#include <vector>

#include "ecs/entity_container.h"

#include "components/virtual_mouse_component.h"
#include "components/network_peer_component.h"
#include "components/transform_component.h"
#include "components/input_component.h"
#include "components/player_controller_component.h"

#include "core/client.h"

ClientPlayerControllerSystem::ClientPlayerControllerSystem()
    : ECS::ISimpleSystem()
{
}

static void ProcessInputs( ECS::EntityContainer& entityContainer, InputState& outInputState )
{
	const InputComponent& inputComponent = entityContainer.GetFirstComponentOfType< InputComponent >();

	outInputState.movement.X( inputComponent.inputController->GetAxis( HORIZONTAL_AXIS ) );
	outInputState.movement.Y( inputComponent.inputController->GetAxis( VERTICAL_AXIS ) );
	outInputState.movement.Normalize();

	const VirtualMouseComponent& virtualMouseComponent =
	    entityContainer.GetFirstComponentOfType< VirtualMouseComponent >();
	outInputState.virtualMousePosition = virtualMouseComponent.position;
}

static void SendInputsToServer( ECS::EntityContainer& entityContainer, const InputState& inputState )
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType< NetworkPeerComponent >();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent< NetworkPeerComponent >();

	NetLib::Client& networkClient = *static_cast< NetLib::Client* >( networkPeerComponent.peer );
	networkClient.SendInputs( inputState );
}

void ClientPlayerControllerSystem::Execute( std::vector< GameEntity >& entities, ECS::EntityContainer& entity_container,
                                            float32 elapsed_time )
{
	const GameEntity networkPeerEntity = entity_container.GetFirstEntityOfType< NetworkPeerComponent >();
	const NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent< NetworkPeerComponent >();
	if ( networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected )
	{
		return;
	}

	InputState inputState;
	ProcessInputs( entity_container, inputState );
	SendInputsToServer( entity_container, inputState );

	// TODO Is this valid?
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		TransformComponent& transform = it->GetComponent< TransformComponent >();
		const PlayerControllerComponent& networkComponent = it->GetComponent< PlayerControllerComponent >();
	}

	// transform.SetPosition(Vec2f(networkComponent.posX.Get(), networkComponent.posY.Get()));
	// transform.SetRotationAngle(networkComponent.rotationAngle.Get());

	// TODO Enable this when client-side prediction and reconciliation is ready
	// TickPlayerController(*it, inputState, elapsedTime);
}
