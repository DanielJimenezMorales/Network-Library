#include "player_state_utils.h"

#include "player_simulation/player_state.h"

#include "ecs/game_entity.hpp"

#include "components/transform_component.h"
#include "components/player_controller_component.h"

void CreatePlayerStateFromPlayerEntity(const ECS::GameEntity& player_entity, PlayerState& player_state)
{
	const TransformComponent& transform = player_entity.GetComponent< TransformComponent >();
	player_state.position = transform.GetPosition();
	player_state.rotationAngle = transform.GetRotationAngle();

	const PlayerControllerComponent& playerController = player_entity.GetComponent< PlayerControllerComponent >();
	player_state.timeLeftUntilNextShot = playerController.timeLeftUntilNextShot;
}

void ApplyPlayerStateToPlayerEntity(ECS::GameEntity& player_entity, const PlayerState& player_state)
{
	TransformComponent& transform = player_entity.GetComponent< TransformComponent >();
	transform.SetPosition(player_state.position);
	transform.SetRotationAngle(player_state.rotationAngle);

	PlayerControllerComponent& playerController = player_entity.GetComponent< PlayerControllerComponent >();
	playerController.timeLeftUntilNextShot = player_state.timeLeftUntilNextShot;
}
