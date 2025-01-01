#pragma once
#include "numeric_types.h"

#include <vector>

class Scene;

namespace ECS
{
	class IFilter;
	class ISimpleSystem;

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

			FilterAndSystemPair( IFilter* filter, ISimpleSystem* system )
			    : filter( filter )
			    , system( system )
			{
			}

			IFilter* filter;
			ISimpleSystem* system;
	};

	class SystemCoordinator
	{
		public:
			SystemCoordinator( ExecutionStage stage );
			// Adds a system to the tail of the vector. The tail is the lowest priority section
			void AddSystemToTail( IFilter* filter, ISimpleSystem* system );
			// Executes all systems in order, from highest to lowest priority
			void Execute( Scene& scene );

		private:
			ExecutionStage _stage;
			std::vector< FilterAndSystemPair > _systemPairs;
	};
} // namespace ECS
