#include "player_network_entity_serialization_callbacks.h"

#include "vec2f.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/transform_component.h"

#include "server/components/server_player_state_storage_component.h"

#include "client/components/client_side_prediction_component.h"

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
	const Engine::TransformComponent& transform = entity.GetComponent< Engine::TransformComponent >();
	const Vec2f position = transform.GetPosition();
	buffer.WriteFloat( position.X() );
	buffer.WriteFloat( position.Y() );
	buffer.WriteFloat( transform.GetRotationAngle() );
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
	Engine::TransformComponent& transform = entity.GetComponent< Engine::TransformComponent >();
	Vec2f position;
	position.X( buffer.ReadFloat() );
	position.Y( buffer.ReadFloat() );

	transform.SetPosition( position );

	const float32 rotation_angle = buffer.ReadFloat();
	transform.SetRotationAngle( rotation_angle );
}
