#pragma once
#include "ecs/i_simple_system.h"

#include "numeric_types.h"

#include "shared/player_simulation/player_state_simulator.h"
#include "shared/player_simulation/player_state_configuration.h"

#include "server/player_simulation/server_player_simulation_events_processor.h"

namespace Engine
{
	namespace ECS
	{
		class Prefab;
	}

	class AssetManager;
}

class InputState;

class ServerPlayerControllerSystem : public Engine::ECS::ISimpleSystem
{
	public:
		ServerPlayerControllerSystem( const Engine::AssetManager* asset_manager );

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab );

	private:
		void ExecutePlayerSimulation( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
		                              const InputState& input_state, float32 elapsed_time, uint32 remote_peer_id );

		const Engine::AssetManager* _assetManager;
		PlayerSimulation::PlayerStateSimulator _playerStateSimulator;
		ServerPlayerSimulationEventsProcessor _eventsProcessor;
};
