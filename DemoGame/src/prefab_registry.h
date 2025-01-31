#pragma once
#include <string>
#include <unordered_map>

#include "ecs/prefab.h"

class PrefabRegistry
{
	public:
		PrefabRegistry();
		PrefabRegistry( const PrefabRegistry& other ) = default;
		PrefabRegistry( PrefabRegistry&& other ) noexcept = default;

		PrefabRegistry& operator=( const PrefabRegistry& other ) = default;
		PrefabRegistry& operator=( PrefabRegistry&& other ) noexcept = default;

		bool RegisterPrefab( const ECS::Prefab& prefab );
		bool UnregisterPrefab( const std::string& name );

		bool TryGetPrefab( const std::string& name, ECS::Prefab& out_prefab ) const;

	private:
		bool IsPrefabValid( const ECS::Prefab& prefab ) const;

		std::unordered_map< std::string, ECS::Prefab > _prefabs;
};
