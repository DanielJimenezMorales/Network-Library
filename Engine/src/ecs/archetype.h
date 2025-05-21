#pragma once
#include <string>
#include <vector>

namespace Engine
{
	namespace ECS
	{
		struct Archetype
		{
				std::string name;
				std::vector< std::string > components;
		};
	}
}
