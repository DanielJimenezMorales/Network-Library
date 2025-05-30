#include "system_coordinator.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "ecs/i_simple_system.h"

#include <cassert>

namespace Engine
{
	namespace ECS
	{
		SystemCoordinator::SystemCoordinator( ExecutionStage stage )
		    : _stage( stage )
		    , _systems()
		{
		}

		ExecutionStage SystemCoordinator::GetStage() const
		{
			return _stage;
		}

		void SystemCoordinator::AddSystemToTail( ISimpleSystem* system )
		{
			assert( system != nullptr );

			_systems.push_back( system );
		}

		void SystemCoordinator::Execute( World& world, float elapsed_time )
		{
			auto system_pairs_it = _systems.begin();
			for ( ; system_pairs_it != _systems.end(); ++system_pairs_it )
			{
				// Execute system
				( *system_pairs_it )->Execute( world, elapsed_time );
			}
		}
	} // namespace ECS
} // namespace Engine
