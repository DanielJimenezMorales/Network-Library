#pragma once
#include "ecs/i_simple_system.h"

#include "ecs/game_entity.hpp"

#include "numeric_types.h"

#include "player_simulation/player_state_simulator.h"
#include "player_simulation/player_state.h"

#include <vector>

namespace ECS
{
	class Prefab;
}

class InputState;
struct ClientSidePredictionComponent;

/// <summary>
/// This system is client-side only.
/// It predicts the local player state based on the input without waiting for receiving the official state from the
/// server. This system helps to minimize latency and increases in-game responsiveness.
/// </summary>
class ClientLocalPlayerPredictorSystem : public ECS::ISimpleSystem
{
	public:
		ClientLocalPlayerPredictorSystem( ECS::World* world );

		void Execute( ECS::World& world, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );
		void ConfigureClientSidePredictorComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

	private:
		void OnShotPerformedCallback();
		void SubscribeToSimulationCallbacks();
		void ExecuteLocalPrediction( ECS::GameEntity& entity, const InputState& input_state, float32 elapsed_time );

		// TODO Remove this world dependency asap
		ECS::World* _world;
		// TODO Remove this variable asap
		ECS::GameEntity _currentPlayerEntityBeingProcessed;

		uint64 _nextInputStateId;

		PlayerStateSimulator _playerStateSimulator;
};
