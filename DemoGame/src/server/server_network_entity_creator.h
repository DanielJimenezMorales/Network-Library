#pragma once
#include "replication/on_network_entity_create_config.h"

namespace Engine
{
	namespace ECS
	{
		class GameEntity;
		class World;
		class Prefab;
	}
}

class ServerNetworkEntityCreator
{
	public:
		ServerNetworkEntityCreator( Engine::ECS::World* world );

		void OnNetworkEntityCreate( const NetLib::OnNetworkEntityCreateConfig& config );
		void OnNetworkEntityDestroy( uint32 in_game_id );
		void OnNetworkEntityComponentConfigure( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab );

	private:
		Engine::ECS::World* _world;
		NetLib::OnNetworkEntityCreateConfig _config;
};
