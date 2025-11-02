#include "client_network_entity_creator.h"
#include <cassert>
#include <string>

#include "vec2f.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "replication/network_entity_communication_callbacks.h"

#include "shared/global_components/network_peer_global_component.h"
#include "shared/components/network_entity_component.h"
#include "shared/player_network_entity_serialization_callbacks.h"
#include "shared/networked_entity_types.h"

#include "client/components/ghost_object_component.h"
#include "client/components/interpolated_object_reference_component.h"

ClientNetworkEntityCreator::ClientNetworkEntityCreator( Engine::ECS::World* world )
    : _world( world )
    , _config()
{
	assert( _world != nullptr );
}

void ClientNetworkEntityCreator::OnNetworkEntityCreate( const NetLib::OnNetworkEntityCreateConfig& config )
{
	static const std::string PLAYER_GHOST_PREFAB_NAME = "ClientPlayerGhost";
	static const std::string PLAYER_INTERPOLATED_PREFAB_NAME = "ClientPlayerInterpolated";
	static const std::string REMOTE_PLAYER_PREFAB_NAME = "RemotePlayer";

	// TODO Add networked entity factories as so far we are only supporting Player networked entity
	ASSERT( config.entityType == PLAYER_NETWORKED_ENTITY_TYPE,
	        "ClientNetworkEntityCreator only supports player entity type creation at the moment." );

	_config = config;
	// Calculate the prefab name of the player it is being created (Local player or remote player)
	const NetworkPeerGlobalComponent& network_peer = _world->GetGlobalComponent< NetworkPeerGlobalComponent >();
	const NetLib::Client* client = network_peer.GetPeerAsClient();
	const std::string prefab_name =
	    client->GetLocalClientId() == config.controlledByPeerId ? PLAYER_GHOST_PREFAB_NAME : REMOTE_PLAYER_PREFAB_NAME;

	// Create the player prefab
	Engine::ECS::GameEntity entity =
	    _world->CreateGameEntity( prefab_name, Vec2f( config.positionX, config.positionY ) );

	// If it is local, not only create the Ghost but also the interpolated object and link them together
	if ( prefab_name == PLAYER_GHOST_PREFAB_NAME )
	{
		Engine::ECS::GameEntity interpolatedEntity =
		    _world->CreateGameEntity( PLAYER_INTERPOLATED_PREFAB_NAME, Vec2f( config.positionX, config.positionY ) );
		GhostObjectComponent& ghostObject = interpolatedEntity.GetComponent< GhostObjectComponent >();
		ghostObject.entity = entity;

		InterpolatedObjectReferenceComponent& interpolatedObjectReference =
		    entity.GetComponent< InterpolatedObjectReferenceComponent >();
		interpolatedObjectReference.entity = interpolatedEntity;
	}
}

void ClientNetworkEntityCreator::OnNetworkEntityDestroy( uint32 network_entity_id )
{
	// If the entity being destroyed is a Client-side player ghost object, do not forget to also remove its interpolated
	// object.
	// TODO Think of a way of making this entire class more flexible and clean. At the moment to many things are
	// hardcoded. Maybe using entity factories to track creation and destruction of entities? Maybe using a new
	// component to identify the type of each entity for easier destruction?
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

	if ( cit->HasComponent< InterpolatedObjectReferenceComponent >() )
	{
		const InterpolatedObjectReferenceComponent& interpolatedObjectReference =
		    cit->GetComponent< InterpolatedObjectReferenceComponent >();
		if ( interpolatedObjectReference.entity.IsValid() )
		{
			_world->DestroyGameEntity( interpolatedObjectReference.entity );
		}
	}

	_world->DestroyGameEntity( *cit );
}

void ClientNetworkEntityCreator::OnNetworkEntityComponentConfigure( Engine::ECS::GameEntity& entity,
                                                                    const Engine::ECS::Prefab& prefab )
{
	if ( !entity.HasComponent< NetworkEntityComponent >() )
	{
		return;
	}

	auto callback_for_owner = [ entity ]( NetLib::Buffer& buffer ) mutable
	{
		DeserializeForOwner( entity, buffer );
	};

	_config.communicationCallbacks->OnUnserializeEntityStateForOwner.AddSubscriber( callback_for_owner );

	auto callback_for_non_owner = [ entity ]( NetLib::Buffer& buffer ) mutable
	{
		DeserializeForNonOwner( entity, buffer );
	};

	_config.communicationCallbacks->OnUnserializeEntityStateForNonOwner.AddSubscriber( callback_for_non_owner );

	NetworkEntityComponent& networkEntity = entity.GetComponent< NetworkEntityComponent >();
	networkEntity.networkEntityId = _config.entityId;
	networkEntity.controlledByPeerId = _config.controlledByPeerId;
}
