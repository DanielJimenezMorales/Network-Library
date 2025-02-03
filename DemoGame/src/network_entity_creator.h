#pragma once
#include "replication/on_network_entity_create_config.h"

#include "core/Peer.h"

class Scene;
class GameEntity;

namespace ECS
{
	class Prefab;
}

class NetworkEntityCreatorSystem
{
	public:
		NetworkEntityCreatorSystem();

		void SetScene( Scene* scene );
		void SetPeerType( NetLib::PeerType peer_type );
		uint32 OnNetworkEntityCreate( const NetLib::OnNetworkEntityCreateConfig& config );
		void OnNetworkEntityDestroy( uint32 in_game_id );
		void OnNetworkEntityComponentConfigure( GameEntity& entity, const ECS::Prefab& prefab );

	private:
		Scene* _scene;
		NetLib::OnNetworkEntityCreateConfig _config;
		NetLib::PeerType _peerType;
};
