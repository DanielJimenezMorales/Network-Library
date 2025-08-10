#pragma once
#include "shared/player_simulation/player_simulation_events.h"

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
	class IPlayerSimulationEventsProcessor
	{
		public:
			IPlayerSimulationEventsProcessor() = default;
			virtual ~IPlayerSimulationEventsProcessor() {};

			virtual bool ProcessEvent( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
			                           PlayerSimulation::EventType type ) = 0;
	};
}
