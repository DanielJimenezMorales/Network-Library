#pragma once
#include "ecs/i_simple_system.h"

#include "ecs/game_entity.hpp"

#include "numeric_types.h"

#include "player_simulation/player_state_simulator.h"
#include "player_simulation/player_state.h"

#include "InputState.h"

#include <vector>

namespace ECS
{
	class World;
	class Prefab;
}

struct ClientSidePredictionComponent;

// TODO Rename this class to something related to Client-side prediction
class ClientPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ClientPlayerControllerSystem( ECS::World* world );

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );
		void ConfigureClientSidePredictorComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

	private:
		void OnShotPerformedCallback();
		void SubscribeToSimulationCallbacks();
		void SavePlayerStateInBuffer( ClientSidePredictionComponent& client_side_prediction_component,
		                              const InputState& input_state, const PlayerState& player_state );

		ECS::World* _world;
		ECS::GameEntity _currentPlayerEntityBeingProcessed;

		uint64 _nextInputStateId;

		PlayerStateSimulator _playerStateSimulator;
};
