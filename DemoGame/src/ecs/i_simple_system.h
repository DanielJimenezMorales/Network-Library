#pragma once
#include "numeric_types.h"

namespace ECS
{
	class GameEntity;
	class World;

	class ISimpleSystem
	{
		public:
			ISimpleSystem() {}
			virtual ~ISimpleSystem() {}

			virtual void Execute( World& world, float32 elapsed_time ) = 0;
	};
}
