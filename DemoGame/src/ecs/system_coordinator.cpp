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

	void SystemCoordinator::AddSystemToTail( IFilter* filter, ISimpleSystem* system )
	{
		assert( filter != nullptr );
		assert( system != nullptr );

		_systemPairs.emplace_back( filter, system );
	}

	void SystemCoordinator::Execute( Scene& scene )
	{
		auto system_pairs_it = _systemPairs.begin();
		for ( ; system_pairs_it != _systemPairs.end(); ++system_pairs_it )
		{
			// Get filtered entities
			const IFilter* filter = system_pairs_it->filter;
			std::vector< GameEntity > filtered_entities = filter->Apply( scene );

			// Execute system with each filtered entity
			auto filtered_entities_it = filtered_entities.begin();
			for ( ; filtered_entities_it != filtered_entities.end(); ++filtered_entities_it )
			{
				GameEntity& filtered_entity = *filtered_entities_it;
				ISimpleSystem* system = system_pairs_it->system;
				system->Execute( filtered_entity );
			}
		}
	}
} // namespace ECS
