#pragma once
#include <vector>

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

		virtual bool LoadArchetypes( std::vector< Engine::ECS::Archetype >& out_archetypes ) = 0;
		virtual bool LoadPrefabs( std::vector< Engine::ECS::Prefab >& out_archetypes ) = 0;
};
