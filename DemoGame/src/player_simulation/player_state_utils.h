#pragma once
#include "numeric_types.h"

namespace ECS
{
	class GameEntity;
}

namespace NetLib
{
	class Buffer;
}

struct PlayerState;

PlayerState GetPlayerStateFromPlayerEntity( const ECS::GameEntity& player_entity, uint32 current_tick );
void ApplyPlayerStateToPlayerEntity( ECS::GameEntity& player_entity, const PlayerState& player_state );

void SerializePlayerStateToBuffer( const PlayerState& player_state, NetLib::Buffer& buffer );
PlayerState DeserializePlayerStateFromBuffer( NetLib::Buffer& buffer );