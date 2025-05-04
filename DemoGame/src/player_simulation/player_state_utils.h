#pragma once
#include "numeric_types.h"

namespace ECS
{
	class GameEntity;
}

struct PlayerState;

PlayerState GetPlayerStateFromPlayerEntity( const ECS::GameEntity& player_entity, uint32 current_tick );
void ApplyPlayerStateToPlayerEntity( ECS::GameEntity& player_entity, const PlayerState& player_state );