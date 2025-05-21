#pragma once
#include "numeric_types.h"

#include <vector>

namespace Engine
{
	namespace ECS
	{
		class ISimpleSystem;
		class World;

		enum class ExecutionStage : uint8
		{
			UPDATE,
			PRETICK,
			TICK,
			POSTICK,
			RENDER
		};

		class SystemCoordinator
		{
			public:
				SystemCoordinator( ExecutionStage stage );

				ExecutionStage GetStage() const;

				// Adds a system to the tail of the vector. The tail is the lowest priority section
				void AddSystemToTail( ISimpleSystem* system );
				// Executes all systems in order, from highest to lowest priority
				void Execute( World& world, float elapsed_time );

			private:
				ExecutionStage _stage;
				std::vector< ISimpleSystem* > _systems;
		};
	} // namespace ECS
} // namespace Engine
