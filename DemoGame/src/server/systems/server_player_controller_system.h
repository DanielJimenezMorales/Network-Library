#pragma once
#include "ecs/i_simple_system.h"

#include "numeric_types.h"

#include "shared/player_simulation/player_state_simulator.h"

#include "server/player_simulation/server_player_simulation_events_processor.h"

namespace Engine
{
	namespace ECS
	{
		class Prefab;
	}
}

class InputState;

class ServerPlayerControllerSystem : public Engine::ECS::ISimpleSystem
{
	public:
		ServerPlayerControllerSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab );

	private:
		void ExecutePlayerSimulation( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
		                              const InputState& input_state, float32 elapsed_time );

		PlayerSimulation::PlayerStateSimulator _playerStateSimulator;
		ServerPlayerSimulationEventsProcessor _eventsProcessor;
};
