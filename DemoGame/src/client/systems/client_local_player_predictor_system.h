#pragma once
#include "ecs/i_simple_system.h"

#include "numeric_types.h"

#include "shared/player_simulation/player_state_simulator.h"

#include "client/player_simulation/client_player_simulation_events_processor.h"

namespace Engine
{
	namespace ECS
	{
		class Prefab;
	}

	class AssetManager;
}

class InputState;
struct ClientSidePredictionComponent;

/// <summary>
/// This system is client-side only.
/// It predicts the local player state based on the input without waiting for receiving the official state from the
/// server. This system helps to minimize latency and increases in-game responsiveness.
/// </summary>
class ClientLocalPlayerPredictorSystem : public Engine::ECS::ISimpleSystem
{
	public:
		ClientLocalPlayerPredictorSystem( const Engine::AssetManager* asset_manager );

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab );
		void ConfigureClientSidePredictorComponent( Engine::ECS::GameEntity& entity,
		                                            const Engine::ECS::Prefab& prefab );

	private:
		void ExecuteLocalPrediction( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
		                             const InputState& input_state, float32 elapsed_time );

		uint64 _nextInputStateId;

		const Engine::AssetManager* _assetManager;
		PlayerSimulation::PlayerStateSimulator _playerStateSimulator;
		ClientPlayerSimulationEventsProcessor _simulationEventsProcessor;
};
