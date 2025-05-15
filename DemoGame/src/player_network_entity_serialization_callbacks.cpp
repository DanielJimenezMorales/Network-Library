#include "player_network_entity_serialization_callbacks.h"

#include "Vec2f.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/transform_component.h"
#include "components/client_side_prediction_component.h"

#include "global_components/network_peer_global_component.h"

#include "core/buffer.h"

#include "player_simulation/player_state.h"
#include "player_simulation/player_state_utils.h"

void SerializeForOwner( const ECS::World& world, const ECS::GameEntity& entity, NetLib::Buffer& buffer )
{
	const NetworkPeerGlobalComponent& networkPeer = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	PlayerState playerState = GetPlayerStateFromPlayerEntity( entity, networkPeer.peer->GetCurrentTick() );
	SerializePlayerStateToBuffer( playerState, buffer );
}

void SerializeForNonOwner( const ECS::GameEntity& entity, NetLib::Buffer& buffer )
{
	const TransformComponent& transform = entity.GetComponent< TransformComponent >();
	const Vec2f position = transform.GetPosition();
	buffer.WriteFloat( position.X() );
	buffer.WriteFloat( position.Y() );
	buffer.WriteFloat( transform.GetRotationAngle() );
}

void DeserializeForOwner( ECS::GameEntity& entity, NetLib::Buffer& buffer )
{
	PlayerState playerState = DeserializePlayerStateFromBuffer( buffer );
	ClientSidePredictionComponent& clientSidePredictionComponent =
	    entity.GetComponent< ClientSidePredictionComponent >();

	clientSidePredictionComponent.playerStatesReceivedFromServer.push_back( playerState );

	ApplyPlayerStateToPlayerEntity( entity, playerState );
}

void DeserializeForNonOwner( ECS::GameEntity& entity, NetLib::Buffer& buffer )
{
	TransformComponent& transform = entity.GetComponent< TransformComponent >();
	Vec2f position;
	position.X( buffer.ReadFloat() );
	position.Y( buffer.ReadFloat() );

	transform.SetPosition( position );

	const float32 rotation_angle = buffer.ReadFloat();
	transform.SetRotationAngle( rotation_angle );
}
