#pragma once
#include <vector>

namespace ECS
{
	struct Archetype;
	class Prefab;
}

class IConfigurationLoader
{
	public:
		virtual ~IConfigurationLoader() {}

		virtual bool LoadArchetypes( std::vector< ECS::Archetype >& out_archetypes ) = 0;
		virtual bool LoadPrefabs( std::vector< ECS::Prefab >& out_archetypes ) = 0;
};
