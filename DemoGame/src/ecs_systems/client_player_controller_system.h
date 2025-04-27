#pragma once
#include "ecs/i_simple_system.h"

#include "player_simulation/player_state_simulator.h"

namespace ECS
{
	class World;
	class Prefab;
}

class ClientPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ClientPlayerControllerSystem( ECS::World* world );

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

	private:
		ECS::World* _world;
};
