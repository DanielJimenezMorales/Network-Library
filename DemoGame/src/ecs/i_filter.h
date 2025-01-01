#pragma once

#include <vector>

class GameEntity;
class Scene;

namespace ECS
{
	class IFilter
	{
		public:
			IFilter() {}
			virtual ~IFilter() {}

			virtual std::vector< GameEntity > Apply( Scene& world ) const = 0;
	};
}
