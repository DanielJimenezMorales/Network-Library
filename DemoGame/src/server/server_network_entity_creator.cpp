#include "server_network_entity_creator.h"
#include <cassert>

#include "vec2f.h"
#include "asserts.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "replication/network_entity_communication_callbacks.h"

#include "shared/components/network_entity_component.h"
#include "shared/player_network_entity_serialization_callbacks.h"
#include "shared/networked_entity_types.h"

ServerNetworkEntityCreator::ServerNetworkEntityCreator( Engine::ECS::World* world )
    : _world( world )
    , _config()
{
	assert( _world != nullptr );
}

void ServerNetworkEntityCreator::OnNetworkEntityCreate( const NetLib::OnNetworkEntityCreateConfig& config )
{
	_config = config;

	ASSERT( config.entityType == PLAYER_NETWORKED_ENTITY_TYPE,
	        "ServerNetworkEntityCreator only supports player entity type creation at the moment." );

	// Create the game entity
	const Engine::ECS::GameEntity entity =
	    _world->CreateGameEntity( "Player", Vec2f( config.positionX, config.positionY ) );
}

void ServerNetworkEntityCreator::OnNetworkEntityDestroy( uint32 network_entity_id )
{
	std::vector< Engine::ECS::GameEntity > networkEntities = _world->GetEntitiesOfType< NetworkEntityComponent >();
	auto cit = networkEntities.cbegin();
	for ( ; cit != networkEntities.cend(); ++cit )
	{
		const NetworkEntityComponent& networkEntityComponent = cit->GetComponent< NetworkEntityComponent >();
		if ( networkEntityComponent.networkEntityId == network_entity_id )
		{
			break;
		}
	}

	// Remove the game entity
	_world->DestroyGameEntity( *cit );
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
