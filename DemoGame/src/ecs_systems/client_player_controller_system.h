#pragma once
#include "ecs/i_simple_system.h"

#include "player_simulation/player_state_simulator.h"
#include "player_simulation/player_state.h"

#include "InputState.h"

#include <vector>

namespace ECS
{
	class World;
	class Prefab;
}

struct PredictionBufferSlot
{
		PredictionBufferSlot()
		    : isValid( false )
		    , inputState()
		    , playerState()
		{
		}

		bool isValid;
		InputState inputState;
		PlayerState playerState;
};

//TODO Rename this class to something related to Client-side prediction
class ClientPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ClientPlayerControllerSystem( ECS::World* world );

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

	private:
		void InitPredictionBuffer();
		void ProcessInputs( ECS::EntityContainer& entityContainer, InputState& outInputState );
		void SavePlayerStateInBuffer( const InputState& input_state, const PlayerState& player_state );

		ECS::World* _world;

		uint64 _nextInputStateId;

		const uint32 MAX_PREDICTION_BUFFER_SIZE = 256;
		std::vector< PredictionBufferSlot > _predictionBuffer;

		PlayerStateSimulator _playerStateSimulator;
};
