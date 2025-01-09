#pragma once
#include "numeric_types.h"

#include <vector>

class GameEntity;

namespace ECS
{
	class EntityContainer;

	class ISimpleSystem
	{
		public:
			ISimpleSystem() {}
			virtual ~ISimpleSystem() {}

			virtual void Execute( std::vector< GameEntity >& entities, EntityContainer& entity_container,
			                      float32 elapsed_time ) = 0;
	};
}
