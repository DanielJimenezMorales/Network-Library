#include "network_entity_creator.h"

#include "Vec2f.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/network_entity_component.h"
#include "global_components/network_peer_global_component.h"
#include "components/ghost_object_component.h"

#include "replication/network_entity_communication_callbacks.h"

#include "player_network_entity_serialization_callbacks.h"

NetworkEntityCreatorSystem::NetworkEntityCreatorSystem()
    : _scene( nullptr )
    , _config()
    , _peerType( NetLib::PeerType::NONE )
{
}

void NetworkEntityCreatorSystem::SetScene( Engine::ECS::World* scene )
{
	_scene = scene;
}

void NetworkEntityCreatorSystem::SetPeerType( NetLib::PeerType peer_type )
{
	_peerType = peer_type;
}

uint32 NetworkEntityCreatorSystem::OnNetworkEntityCreate( const NetLib::OnNetworkEntityCreateConfig& config )
{
	_config = config;

	std::string prefab_name;
	const NetworkPeerGlobalComponent& network_peer = _scene->GetGlobalComponent< NetworkPeerGlobalComponent >();
	if ( _peerType == NetLib::PeerType::CLIENT )
	{
		const NetLib::Client* client = network_peer.GetPeerAsClient();
		if ( client->GetLocalClientId() == config.controlledByPeerId )
		{
			prefab_name.assign( "ClientPlayerGhost" );
		}
		else
		{
			prefab_name.assign( "RemotePlayer" );
		}
	}
	else if ( _peerType == NetLib::PeerType::SERVER )
	{
		prefab_name.assign( "Player" );
	}

	const Engine::ECS::GameEntity entity =
	    _scene->CreateGameEntity( prefab_name, Vec2f( config.positionX, config.positionY ) );

	if ( prefab_name == "ClientPlayerGhost" )
	{
		Engine::ECS::GameEntity interpolatedEntity =
		    _scene->CreateGameEntity( "ClientPlayerInterpolated", Vec2f( config.positionX, config.positionY ) );
		GhostObjectComponent& ghostObject = interpolatedEntity.GetComponent< GhostObjectComponent >();
		ghostObject.entity = entity;
	}

	return entity.GetId();
}

void NetworkEntityCreatorSystem::OnNetworkEntityDestroy( uint32 in_game_id )
{
	_scene->DestroyGameEntity( in_game_id );
}

void NetworkEntityCreatorSystem::OnNetworkEntityComponentConfigure( Engine::ECS::GameEntity& entity,
                                                                    const Engine::ECS::Prefab& prefab )
{
	if ( !entity.HasComponent< NetworkEntityComponent >() )
	{
		return;
	}

	if ( _peerType == NetLib::PeerType::CLIENT )
	{
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
	}
	else if ( _peerType == NetLib::PeerType::SERVER )
	{
		const Engine::ECS::World& world = *_scene;
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
	}

	NetworkEntityComponent& network_entity = entity.GetComponent< NetworkEntityComponent >();
	network_entity.networkEntityId = _config.entityId;
	network_entity.controlledByPeerId = _config.controlledByPeerId;
}
