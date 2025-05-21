#pragma once
#include "replication/on_network_entity_create_config.h"

#include "core/Peer.h"

namespace Engine
{
	namespace ECS
	{
		class GameEntity;
		class World;
		class Prefab;
	}
}

class NetworkEntityCreatorSystem
{
	public:
		NetworkEntityCreatorSystem();

		void SetScene( Engine::ECS::World* scene );
		void SetPeerType( NetLib::PeerType peer_type );
		uint32 OnNetworkEntityCreate( const NetLib::OnNetworkEntityCreateConfig& config );
		void OnNetworkEntityDestroy( uint32 in_game_id );
		void OnNetworkEntityComponentConfigure( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab );

	private:
		Engine::ECS::World* _scene;
		NetLib::OnNetworkEntityCreateConfig _config;
		NetLib::PeerType _peerType;
};
