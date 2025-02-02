#include "network_entity_creator.h"

#include "Scene.h"
#include "GameEntity.hpp"
#include "Vec2f.h"

#include "components/network_entity_component.h"
#include "global_components/network_peer_global_component.h"

#include "replication/network_entity_communication_callbacks.h"

#include "player_network_entity_serialization_callbacks.h"

NetworkEntityCreatorSystem::NetworkEntityCreatorSystem()
    : _scene( nullptr )
    , _config()
    , _peerType( NetLib::PeerType::NONE )
{
}

void NetworkEntityCreatorSystem::SetScene( Scene* scene )
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
			prefab_name.assign( "Player" );
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

	const GameEntity entity = _scene->CreateGameEntity( prefab_name, Vec2f( config.positionX, config.positionY ) );
	return entity.GetId();
}

void NetworkEntityCreatorSystem::OnNetworkEntityComponentConfigure( GameEntity& entity, const ECS::Prefab& prefab )
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
	}
	else if ( _peerType == NetLib::PeerType::SERVER )
	{
		auto callback_for_owner = [ entity ]( NetLib::Buffer& buffer ) mutable
		{
			SerializeForOwner( entity, buffer );
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
