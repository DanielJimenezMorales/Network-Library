#pragma once
#include <vector>
#include <string>

namespace Engine
{
	namespace ECS
	{
		struct Archetype;
		class Prefab;
	}
}

class IConfigurationLoader
{
	public:
		virtual ~IConfigurationLoader() {}

		virtual bool LoadArchetypes( std::vector< Engine::ECS::Archetype >& out_archetypes,
		                             const std::string& relative_path ) = 0;
		virtual bool LoadPrefabs( std::vector< Engine::ECS::Prefab >& out_archetypes,
		                          const std::string& relative_path ) = 0;
};
