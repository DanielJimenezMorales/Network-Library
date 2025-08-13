#pragma once
#include "shared/player_simulation/i_simulation_events_processor.h"

class ClientPlayerSimulationEventsProcessor : public PlayerSimulation::ISimulationEventsProcessor
{
	public:
		ClientPlayerSimulationEventsProcessor() = default;

		bool ProcessEvent( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
		                   PlayerSimulation::EventType type ) override;
};