#include "server_network_entity_creator.h"
#include <cassert>

#include "vec2f.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "replication/network_entity_communication_callbacks.h"

#include "shared/components/network_entity_component.h"
#include "shared/player_network_entity_serialization_callbacks.h"

ServerNetworkEntityCreator::ServerNetworkEntityCreator( Engine::ECS::World* world )
    : _world( world )
    , _config()
{
	assert( _world != nullptr );
}

uint32 ServerNetworkEntityCreator::OnNetworkEntityCreate( const NetLib::OnNetworkEntityCreateConfig& config )
{
	_config = config;

	// Create the game entity
	const Engine::ECS::GameEntity entity =
	    _world->CreateGameEntity( "Player", Vec2f( config.positionX, config.positionY ) );

	return entity.GetId();
}

void ServerNetworkEntityCreator::OnNetworkEntityDestroy( uint32 in_game_id )
{
	const Engine::ECS::GameEntity entity = _world->GetEntityFromId( in_game_id );
	const NetworkEntityComponent& networkEntityComponent = entity.GetComponent< NetworkEntityComponent >();

	// Remove the game entity
	_world->DestroyGameEntity( in_game_id );
}

void ServerNetworkEntityCreator::OnNetworkEntityComponentConfigure( Engine::ECS::GameEntity& entity,
                                                                    const Engine::ECS::Prefab& prefab )
{
	if ( !entity.HasComponent< NetworkEntityComponent >() )
	{
		return;
	}

	const Engine::ECS::World& world = *_world;
	auto callback_for_owner = [ &world, entity ]( NetLib::Buffer& buffer ) mutable
	{
		SerializeForOwner( world, entity, buffer );
	};

	_config.communicationCallbacks->OnSerializeEntityStateForOwner.AddSubscriber( callback_for_owner );

	auto callback_for_non_owner = [ entity ]( NetLib::Buffer& buffer ) mutable
	{
		SerializeForNonOwner( entity, buffer );
	};

	_config.communicationCallbacks->OnSerializeEntityStateForNonOwner.AddSubscriber( callback_for_non_owner );

	NetworkEntityComponent& networkEntity = entity.GetComponent< NetworkEntityComponent >();
	networkEntity.networkEntityId = _config.entityId;
	networkEntity.controlledByPeerId = _config.controlledByPeerId;
}
