#pragma once

#include <vector>

namespace Engine
{
	namespace ECS
	{
		class GameEntity;
		class EntityContainer;

		class IFilter
		{
			public:
				IFilter() {}
				virtual ~IFilter() {}

				virtual std::vector< GameEntity > Apply( EntityContainer& entity_container ) const = 0;
		};
	}
}
