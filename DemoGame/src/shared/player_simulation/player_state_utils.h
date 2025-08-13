#pragma once
#include "numeric_types.h"

namespace Engine
{
	namespace ECS
	{
		class GameEntity;
	}
}

namespace NetLib
{
	class Buffer;
}

namespace PlayerSimulation
{
	struct PlayerState;

	PlayerState GetPlayerStateFromPlayerEntity( const Engine::ECS::GameEntity& player_entity, uint32 current_tick );
	void ApplyPlayerStateToPlayerEntity( Engine::ECS::GameEntity& player_entity, const PlayerState& player_state );

	void SerializePlayerStateToBuffer( const PlayerState& player_state, NetLib::Buffer& buffer );
	PlayerState DeserializePlayerStateFromBuffer( NetLib::Buffer& buffer );
}