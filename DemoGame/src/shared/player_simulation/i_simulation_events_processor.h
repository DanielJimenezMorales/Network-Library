#pragma once
#include "shared/player_simulation/simulation_events.h"

namespace Engine
{
	namespace ECS
	{
		class World;
		class GameEntity;
	}
}

namespace PlayerSimulation
{
	class ISimulationEventsProcessor
	{
		public:
			ISimulationEventsProcessor() = default;
			virtual ~ISimulationEventsProcessor() {};

			virtual bool ProcessEvent( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
			                           EventType type ) = 0;
	};
}
