#include "player_state_utils.h"

#include "shared/player_simulation/player_state.h"

#include "ecs/game_entity.hpp"

#include "components/transform_component.h"

#include "shared/components/player_controller_component.h"

#include "core/Buffer.h"

namespace PlayerSimulation
{
	PlayerState GetPlayerStateFromPlayerEntity( const Engine::ECS::GameEntity& player_entity, uint32 current_tick )
	{
		PlayerState playerState;
		playerState.tick = current_tick;

		const Engine::TransformComponent& transform = player_entity.GetComponent< Engine::TransformComponent >();
		playerState.position = transform.GetPosition();
		playerState.rotationAngle = transform.GetRotationAngle();

		const PlayerControllerComponent& playerController = player_entity.GetComponent< PlayerControllerComponent >();
		playerState.timeLeftUntilNextShot = playerController.timeLeftUntilNextShot;

		return playerState;
	}

	void ApplyPlayerStateToPlayerEntity( Engine::ECS::GameEntity& player_entity, const PlayerState& player_state )
	{
		// Update Transform
		Engine::TransformComponent& transform = player_entity.GetComponent< Engine::TransformComponent >();
		transform.SetPosition( player_state.position );
		transform.SetRotationAngle( player_state.rotationAngle );

		// Update Player Controller
		PlayerControllerComponent& playerController = player_entity.GetComponent< PlayerControllerComponent >();
		playerController.timeLeftUntilNextShot = player_state.timeLeftUntilNextShot;
	}

	void SerializePlayerStateToBuffer( const PlayerState& player_state, NetLib::Buffer& buffer )
	{
		buffer.WriteInteger( player_state.tick );
		buffer.WriteFloat( player_state.position.X() );
		buffer.WriteFloat( player_state.position.Y() );

		buffer.WriteFloat( player_state.rotationAngle );
	}

	PlayerState DeserializePlayerStateFromBuffer( NetLib::Buffer& buffer )
	{
		PlayerState playerState;

		playerState.tick = buffer.ReadInteger();
		playerState.position.X( buffer.ReadFloat() );
		playerState.position.Y( buffer.ReadFloat() );

		playerState.rotationAngle = buffer.ReadFloat();

		return playerState;
	}
} // namespace PlayerSimulation
