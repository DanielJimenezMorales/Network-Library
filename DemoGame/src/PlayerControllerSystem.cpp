#include "PlayerControllerSystem.h"
#include "GameEntity.hpp"
#include "InputComponent.h"
#include "IInputController.h"
#include "ICursor.h"
#include "Vec2f.h"
#include "TransformComponent.h"
#include "PlayerControllerComponent.h"
#include "InputActionIdsConfiguration.h"
#include "InputState.h"
#include "NetworkPeerComponent.h"
#include "VirtualMouseComponent.h"

#include <vector>

#include "ecs/entity_container.h"

#include "core/client.h"

PlayerControllerSystem::PlayerControllerSystem()
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

void PlayerControllerSystem::Execute( GameEntity& entity, float32 elapsed_time )
{
	ECS::EntityContainer* entity_container = entity.GetEntityContainer();

	const GameEntity networkPeerEntity = entity_container->GetFirstEntityOfType< NetworkPeerComponent >();
	const NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent< NetworkPeerComponent >();
	if ( networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected )
	{
		return;
	}

	InputState inputState;
	ProcessInputs( *entity_container, inputState );
	SendInputsToServer( *entity_container, inputState );

	TransformComponent& transform = entity.GetComponent< TransformComponent >();
	const PlayerControllerComponent& networkComponent = entity.GetComponent< PlayerControllerComponent >();

	// transform.SetPosition(Vec2f(networkComponent.posX.Get(), networkComponent.posY.Get()));
	// transform.SetRotationAngle(networkComponent.rotationAngle.Get());

	// TODO Enable this when client-side prediction and reconciliation is ready
	// TickPlayerController(*it, inputState, elapsedTime);
}
