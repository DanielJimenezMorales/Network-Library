#pragma once
#include <string>
#include <unordered_map>

#include "ecs/archetype.h"

class ArchetypeRegistry
{
	public:
		ArchetypeRegistry();
		ArchetypeRegistry( const ArchetypeRegistry& other ) = default;
		ArchetypeRegistry( ArchetypeRegistry&& other ) noexcept = default;

		ArchetypeRegistry& operator=( const ArchetypeRegistry& other ) = default;
		ArchetypeRegistry& operator=( ArchetypeRegistry&& other ) noexcept = default;

		bool RegisterArchetype( const ECS::Archetype& archetype );
		bool UnregisterArchetype( const std::string& name );

		bool TryGetArchetype( const std::string& name, ECS::Archetype& out_archetype ) const;

	private:
		bool IsArchetypeValid( const ECS::Archetype& archetype ) const;

		std::unordered_map< std::string, ECS::Archetype > _archetypes;
};
