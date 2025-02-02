#include "prefab_registry.h"

#include "logger.h"

PrefabRegistry::PrefabRegistry()
    : _prefabs()
{
}

bool PrefabRegistry::RegisterPrefab( ECS::Prefab&& prefab )
{
	bool result = false;
	auto prefab_found = _prefabs.find( prefab.name );
	if ( prefab_found == _prefabs.end() )
	{
		_prefabs[ prefab.name ] = std::move( prefab );
		result = true;
	}
	else
	{
		LOG_ERROR( "[PrefabRegistry::RegisterPrefab] Archetype with name %s has already been registered",
		           prefab.name.c_str() );
	}

	return result;
}

bool PrefabRegistry::UnregisterPrefab( const std::string& name )
{
	bool result = false;
	const size_t items_removed = _prefabs.erase( name );
	if ( items_removed == 1 )
	{
		result = true;
	}
	else
	{
		LOG_ERROR( "[PrefabRegistry::UnregisterPrefab] Can't unregister prefab with name %s because it wasn't "
		           "registered",
		           name.c_str() );
	}

	return result;
}

const ECS::Prefab* PrefabRegistry::TryGetPrefab( const std::string& name ) const
{
	const ECS::Prefab* result = nullptr;
	auto prefab_found = _prefabs.find( name );
	if ( prefab_found != _prefabs.end() )
	{
		result = &prefab_found->second;
	}

	return result;
}
