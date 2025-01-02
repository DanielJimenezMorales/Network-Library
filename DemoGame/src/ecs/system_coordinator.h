#pragma once
#include "numeric_types.h"

#include <vector>

namespace ECS
{
	class IFilter;
	class ISimpleSystem;
	class EntityContainer;

	enum class ExecutionStage : uint8
	{
		UPDATE,
		PRETICK,
		TICK,
		POSTICK
	};

	struct FilterAndSystemPair
	{
			FilterAndSystemPair()
			    : filter( nullptr )
			    , system( nullptr )
			{
			}

			FilterAndSystemPair( const IFilter* filter, ISimpleSystem* system )
			    : filter( filter )
			    , system( system )
			{
			}

			const IFilter* filter;
			ISimpleSystem* system;
	};

	class SystemCoordinator
	{
		public:
			SystemCoordinator( ExecutionStage stage );

			ExecutionStage GetStage() const;

			// Adds a system to the tail of the vector. The tail is the lowest priority section
			void AddSystemToTail( const IFilter* filter, ISimpleSystem* system );
			// Executes all systems in order, from highest to lowest priority
			void Execute( EntityContainer& entity_container, float elapsed_time );

		private:
			ExecutionStage _stage;
			std::vector< FilterAndSystemPair > _systemPairs;
	};
} // namespace ECS
