#pragma once

namespace ECS
{
	class GameEntity;
}

struct PlayerState;

void CreatePlayerStateFromPlayerEntity(const ECS::GameEntity& player_entity, PlayerState& player_state);
void ApplyPlayerStateToPlayerEntity(ECS::GameEntity& player_entity, const PlayerState& player_state);