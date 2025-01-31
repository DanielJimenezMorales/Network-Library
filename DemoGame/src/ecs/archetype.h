#pragma once
#include <string>
#include <vector>

namespace ECS
{
	class Archetype
	{
		public:
			std::string name;
			std::vector< std::string > components;
	};
}
