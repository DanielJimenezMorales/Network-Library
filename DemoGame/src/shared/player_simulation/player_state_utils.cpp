#include "player_state_utils.h"

#include "shared/player_simulation/player_state.h"

#include "ecs/game_entity.hpp"

#include "transform/transform_component.h"
#include "transform/transform_hierarchy_helper_functions.h"

#include "shared/components/player_controller_component.h"

#include "core/Buffer.h"

namespace PlayerSimulation
{
	PlayerState GetPlayerStateFromPlayerEntity( const Engine::ECS::GameEntity& player_entity, uint32 current_tick )
	{
		PlayerState playerState;
		playerState.tick = current_tick;

		const Engine::TransformComponentProxy transformComponentProxy;
		const Engine::TransformComponent& transform = player_entity.GetComponent< Engine::TransformComponent >();
		playerState.position = transformComponentProxy.GetGlobalPosition( transform );
		playerState.rotationAngle = transformComponentProxy.GetGlobalRotation( transform );

		const PlayerControllerComponent& playerController = player_entity.GetComponent< PlayerControllerComponent >();
		playerState.movementDirection = playerController.state.movementDirection;
		playerState.isWalking = playerController.state.isWalking;
		playerState.isAiming = playerController.state.isAiming;
		playerState.timeLeftUntilNextShot = playerController.state.timeLeftUntilNextShot;

		return playerState;
	}

	void ApplyPlayerStateToPlayerEntity( Engine::ECS::GameEntity& player_entity, const PlayerState& player_state )
	{
		const Engine::TransformComponentProxy transformComponentProxy;
		// Update Transform
		Engine::TransformComponent& transform = player_entity.GetComponent< Engine::TransformComponent >();
		transformComponentProxy.SetGlobalPosition( transform, player_state.position );
		transformComponentProxy.SetGlobalRotationAngle( transform, player_state.rotationAngle );

		// Update Player Controller
		PlayerControllerComponent& playerController = player_entity.GetComponent< PlayerControllerComponent >();
		playerController.state = player_state;
	}

	void SerializePlayerStateToBuffer( const PlayerState& player_state, NetLib::Buffer& buffer )
	{
		buffer.WriteInteger( player_state.tick );
		buffer.WriteFloat( player_state.position.X() );
		buffer.WriteFloat( player_state.position.Y() );

		buffer.WriteFloat( player_state.rotationAngle );
		buffer.WriteFloat( player_state.timeLeftUntilNextShot );

		buffer.WriteFloat( player_state.movementDirection.X() );
		buffer.WriteFloat( player_state.movementDirection.Y() );

		// TODO Serialize this in a more efficient way to fit multiple bools in one byte
		buffer.WriteInteger( player_state.isWalking ? 1 : 0 );
		buffer.WriteInteger( player_state.isAiming ? 1 : 0 );
	}

	PlayerState DeserializePlayerStateFromBuffer( NetLib::Buffer& buffer )
	{
		PlayerState playerState;

		playerState.tick = buffer.ReadInteger();
		playerState.position.X( buffer.ReadFloat() );
		playerState.position.Y( buffer.ReadFloat() );

		playerState.rotationAngle = buffer.ReadFloat();
		playerState.timeLeftUntilNextShot = buffer.ReadFloat();

		playerState.movementDirection.X( buffer.ReadFloat() );
		playerState.movementDirection.Y( buffer.ReadFloat() );

		// TODO Serialize this in a more efficient way to fit multiple bools in one byte
		playerState.isWalking = buffer.ReadInteger() == 1 ? true : false;
		playerState.isAiming = buffer.ReadInteger() == 1 ? true : false;

		return playerState;
	}
} // namespace PlayerSimulation
