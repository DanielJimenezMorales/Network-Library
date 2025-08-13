#pragma once
#include "shared/player_simulation/simulation_events.h"

#include <vector>

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
	class ISimulationEventsProcessor;

	class SimulationEventsHandler
	{
		public:
			SimulationEventsHandler() = default;

			void AddEvent( EventType event );
			void Clear();
			void ProcessEvents( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
			                    ISimulationEventsProcessor* events_processor ) const;

		private:
			std::vector< EventType > _events;
	};
} // namespace PlayerSimulation
