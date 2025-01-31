#pragma once
#include <string>
#include <vector>

#include "ecs/component_configuration.h"

namespace ECS
{
	class Prefab
	{
		public:
			std::string name;
			std::string archetype;
			std::vector< ComponentConfiguration > componentConfigurations;
	};
}
