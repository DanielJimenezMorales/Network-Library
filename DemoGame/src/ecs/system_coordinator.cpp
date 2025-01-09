#include "system_coordinator.h"

#include "GameEntity.hpp"

#include "i_filter.h"
#include "i_simple_system.h"

#include <cassert>

namespace ECS
{
	SystemCoordinator::SystemCoordinator( ExecutionStage stage )
	    : _stage( stage )
	    , _systemPairs()
	{
	}

	ExecutionStage SystemCoordinator::GetStage() const
	{
		return _stage;
	}

	void SystemCoordinator::AddSystemToTail( const IFilter* filter, ISimpleSystem* system )
	{
		assert( filter != nullptr );
		assert( system != nullptr );

		_systemPairs.emplace_back( filter, system );
	}

	void SystemCoordinator::Execute( EntityContainer& entity_container, float elapsed_time )
	{
		auto system_pairs_it = _systemPairs.begin();
		for ( ; system_pairs_it != _systemPairs.end(); ++system_pairs_it )
		{
			// Get filtered entities
			const IFilter* filter = system_pairs_it->filter;
			std::vector< GameEntity > filtered_entities = filter->Apply( entity_container );

			if ( !filtered_entities.empty() )
			{
				// Execute system
				ISimpleSystem* system = system_pairs_it->system;
				system->Execute( filtered_entities, entity_container, elapsed_time );
			}
		}
	}
} // namespace ECS
