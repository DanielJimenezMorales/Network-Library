#include "archetype_registry.h"

#include "logger.h"

#include <cassert>

ArchetypeRegistry::ArchetypeRegistry()
    : _archetypes()
{
}

bool ArchetypeRegistry::RegisterArchetype( const ECS::Archetype& archetype )
{
	assert( IsArchetypeValid( archetype ) );

	bool result = false;
	auto archetype_found = _archetypes.find( archetype.name );
	if ( archetype_found == _archetypes.end() )
	{
		_archetypes[ archetype.name ] = archetype;
		result = true;
	}
	else
	{
		LOG_ERROR( "[ArchetypeRegistry::RegisterArchetype] Archetype with name %s has already been registered",
		           archetype.name.c_str() );
	}

	return result;
}

bool ArchetypeRegistry::UnregisterArchetype( const std::string& name )
{
	bool result = false;
	const size_t items_removed = _archetypes.erase( name );
	if ( items_removed == 1 )
	{
		result = true;
	}
	else
	{
		LOG_ERROR( "[ArchetypeRegistry::UnregisterArchetype] Can't unregister archetype with name %s because it wasn't "
		           "registered",
		           name.c_str() );
	}

	return result;
}

bool ArchetypeRegistry::TryGetArchetype( const std::string& name, ECS::Archetype& out_archetype ) const
{
	bool result = false;
	auto archetype_found = _archetypes.find( name );
	if ( archetype_found != _archetypes.end() )
	{
		out_archetype = archetype_found->second;
		result = true;
	}

	return result;
}

bool ArchetypeRegistry::IsArchetypeValid( const ECS::Archetype& archetype ) const
{
	return archetype.name.empty() || archetype.components.empty();
}
