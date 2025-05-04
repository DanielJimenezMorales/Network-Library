#include "player_state_utils.h"

#include "player_simulation/player_state.h"

#include "ecs/game_entity.hpp"

#include "components/transform_component.h"
#include "components/player_controller_component.h"

PlayerState GetPlayerStateFromPlayerEntity( const ECS::GameEntity& player_entity, uint32 current_tick )
{
	PlayerState playerState;
	playerState.tick = current_tick;

	const TransformComponent& transform = player_entity.GetComponent< TransformComponent >();
	playerState.position = transform.GetPosition();
	playerState.rotationAngle = transform.GetRotationAngle();

	const PlayerControllerComponent& playerController = player_entity.GetComponent< PlayerControllerComponent >();
	playerState.timeLeftUntilNextShot = playerController.timeLeftUntilNextShot;

	return playerState;
}

void ApplyPlayerStateToPlayerEntity( ECS::GameEntity& player_entity, const PlayerState& player_state )
{
	TransformComponent& transform = player_entity.GetComponent< TransformComponent >();
	transform.SetPosition( player_state.position );
	transform.SetRotationAngle( player_state.rotationAngle );

	PlayerControllerComponent& playerController = player_entity.GetComponent< PlayerControllerComponent >();
	playerController.timeLeftUntilNextShot = player_state.timeLeftUntilNextShot;
}
