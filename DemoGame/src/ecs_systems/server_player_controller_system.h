#pragma once
#include "ecs/i_simple_system.h"

#include "numeric_types.h"

#include "player_simulation/player_state_simulator.h"

namespace ECS
{
	class Prefab;
	class World;
}

class ServerPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ServerPlayerControllerSystem( ECS::World* world );

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

	private:
		PlayerStateSimulator _playerStateSimulator;
};
