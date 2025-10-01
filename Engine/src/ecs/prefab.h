#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "numeric_types.h"
#include "vec2f.h"

#include "ecs/component_configuration.h"

namespace Engine
{
	namespace ECS
	{
		struct ChildPrefab
		{
				std::string name;
				Vec2f localPosition;
				float32 localRotation;
				Vec2f localScale;
		};

		class Prefab
		{
			public:
				Prefab();
				Prefab( const Prefab& other );
				Prefab( Prefab&& other ) noexcept = default;

				~Prefab();

				Prefab& operator=( const Prefab& other );
				Prefab& operator=( Prefab&& other ) noexcept = default;

				std::string name;
				std::string archetype;
				std::unordered_map< std::string, ComponentConfiguration* > componentConfigurations;
				std::vector< ChildPrefab > childrenPrefabs;

			private:
				void Free();
				void CopyComponents( const std::unordered_map< std::string, ComponentConfiguration* >& other );
		};
	} // namespace ECS
} // namespace Engine
