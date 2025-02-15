#pragma once
#include "replication/on_network_entity_create_config.h"

#include "core/Peer.h"

namespace ECS
{
	class GameEntity;
	class Scene;
	class Prefab;
}

class NetworkEntityCreatorSystem
{
	public:
		NetworkEntityCreatorSystem();

		void SetScene( ECS::Scene* scene );
		void SetPeerType( NetLib::PeerType peer_type );
		uint32 OnNetworkEntityCreate( const NetLib::OnNetworkEntityCreateConfig& config );
		void OnNetworkEntityDestroy( uint32 in_game_id );
		void OnNetworkEntityComponentConfigure( ECS::GameEntity& entity, const ECS::Prefab& prefab );

	private:
		ECS::Scene* _scene;
		NetLib::OnNetworkEntityCreateConfig _config;
		NetLib::PeerType _peerType;
};
