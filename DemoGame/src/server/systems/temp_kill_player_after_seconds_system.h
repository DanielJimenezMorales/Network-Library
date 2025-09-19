#pragma once
#include "ecs/i_simple_system.h"

#include "numeric_types.h"


class TempKillPlayerAfterSecondsSystem : public Engine::ECS::ISimpleSystem
{
	public:
		TempKillPlayerAfterSecondsSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;

		void OnPlayerCreated( Engine::ECS::GameEntity& entity );

	private:
		bool _startCounter;
		float32 _counter;
};
