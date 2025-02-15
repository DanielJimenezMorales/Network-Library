#pragma once
#include <string>
#include <vector>

namespace ECS
{
	struct Archetype
	{
			std::string name;
			std::vector< std::string > components;
	};
}
