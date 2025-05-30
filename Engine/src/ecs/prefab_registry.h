#pragma once
#include <string>
#include <unordered_map>

#include "ecs/prefab.h"

namespace Engine
{
	namespace ECS
	{
		class PrefabRegistry
		{
			public:
				PrefabRegistry();
				PrefabRegistry( const PrefabRegistry& other ) = default;
				PrefabRegistry( PrefabRegistry&& other ) noexcept = default;

				PrefabRegistry& operator=( const PrefabRegistry& other ) = default;
				PrefabRegistry& operator=( PrefabRegistry&& other ) noexcept = default;

				bool RegisterPrefab( Prefab&& prefab );
				bool UnregisterPrefab( const std::string& name );

				const ECS::Prefab* TryGetPrefab( const std::string& name ) const;

			private:
				bool IsPrefabValid( const Prefab& prefab ) const;

				std::unordered_map< std::string, Prefab > _prefabs;
		};
	}
}
