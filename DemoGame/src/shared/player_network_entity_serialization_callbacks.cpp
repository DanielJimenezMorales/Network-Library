#include "player_network_entity_serialization_callbacks.h"

#include "vec2f.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "server/components/server_player_state_storage_component.h"

#include "client/components/client_side_prediction_component.h"

#include "client/player_interpolation/player_interpolated_state_component.h"

#include "shared/global_components/network_peer_global_component.h"

#include "core/buffer.h"

#include "shared/player_simulation/player_state.h"
#include "shared/player_simulation/player_state_utils.h"

void SerializeForOwner( const Engine::ECS::World& world, const Engine::ECS::GameEntity& entity, NetLib::Buffer& buffer )
{
	const ServerPlayerStateStorageComponent& serverPlayerStateStorage =
	    entity.GetComponent< ServerPlayerStateStorageComponent >();
	SerializePlayerStateToBuffer( serverPlayerStateStorage.lastPlayerStateSimulated, buffer );
}

void SerializeForNonOwner( const Engine::ECS::GameEntity& entity, NetLib::Buffer& buffer )
{
	const ServerPlayerStateStorageComponent& serverPlayerStateStorage =
	    entity.GetComponent< ServerPlayerStateStorageComponent >();

	buffer.WriteFloat( serverPlayerStateStorage.lastPlayerStateSimulated.position.X() );
	buffer.WriteFloat( serverPlayerStateStorage.lastPlayerStateSimulated.position.Y() );
	buffer.WriteFloat( serverPlayerStateStorage.lastPlayerStateSimulated.rotationAngle );
	buffer.WriteFloat( serverPlayerStateStorage.lastPlayerStateSimulated.movementDirection.X() );
	buffer.WriteFloat( serverPlayerStateStorage.lastPlayerStateSimulated.movementDirection.Y() );

	// TODO Serialize this in a more efficient way to fit multiple bools in one byte
	buffer.WriteInteger( serverPlayerStateStorage.lastPlayerStateSimulated.isWalking ? 1 : 0 );
	buffer.WriteInteger( serverPlayerStateStorage.lastPlayerStateSimulated.isAiming ? 1 : 0 );
}

void DeserializeForOwner( Engine::ECS::GameEntity& entity, NetLib::Buffer& buffer )
{
	PlayerSimulation::PlayerState playerState = PlayerSimulation::DeserializePlayerStateFromBuffer( buffer );
	ClientSidePredictionComponent& clientSidePredictionComponent =
	    entity.GetComponent< ClientSidePredictionComponent >();

	clientSidePredictionComponent.playerStatesReceivedFromServer.push_back( playerState );
	clientSidePredictionComponent.isPendingPlayerStateFromServer = true;
}

void DeserializeForNonOwner( Engine::ECS::GameEntity& entity, NetLib::Buffer& buffer )
{
	PlayerInterpolatedStateComponent& interpolatedStateComponent =
	    entity.GetComponent< PlayerInterpolatedStateComponent >();

	PlayerInterpolatedState newState;
	newState.position.X( buffer.ReadFloat() );
	newState.position.Y( buffer.ReadFloat() );
	newState.rotationAngle = buffer.ReadFloat();
	newState.movementDirection.X( buffer.ReadFloat() );
	newState.movementDirection.Y( buffer.ReadFloat() );

	// TODO Serialize this in a more efficient way to fit multiple bools in one byte
	newState.isWalking = buffer.ReadInteger() == 1 ? true : false;
	newState.isAiming = buffer.ReadInteger() == 1 ? true : false;

	interpolatedStateComponent.state = newState;
}
