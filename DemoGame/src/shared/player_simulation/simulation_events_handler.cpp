#include "simulation_events_handler.h"

#include "logger.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "shared/player_simulation/i_player_simulation_events_processor.h"

#include <cassert>

namespace PlayerSimulation
{
	void SimulationEventsHandler::AddEvent( EventType event )
	{
		_events.push_back( event );
	}

	void SimulationEventsHandler::Clear()
	{
		_events.clear();
	}

	void SimulationEventsHandler::ProcessEvents( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
	                                             IPlayerSimulationEventsProcessor* events_processor ) const
	{
		assert( entity.IsValid() );
		assert( events_processor != nullptr );

		for ( auto cit = _events.cbegin(); cit != _events.cend(); ++cit )
		{
			const bool result = events_processor->ProcessEvent( world, entity, *cit );
			if ( !result )
			{
				LOG_ERROR( "Failed to process simulation event of type %u", *cit );
			}
		}
	}
} // namespace PlayerSimulation
