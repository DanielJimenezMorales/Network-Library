#pragma once
#include <string>
#include <unordered_map>

#include "ecs/archetype.h"

namespace Engine
{
	namespace ECS
	{
		class ArchetypeRegistry
		{
			public:
				ArchetypeRegistry();
				ArchetypeRegistry( const ArchetypeRegistry& other ) = default;
				ArchetypeRegistry( ArchetypeRegistry&& other ) noexcept = default;

				ArchetypeRegistry& operator=( const ArchetypeRegistry& other ) = default;
				ArchetypeRegistry& operator=( ArchetypeRegistry&& other ) noexcept = default;

				bool RegisterArchetype( const Archetype& archetype );
				bool UnregisterArchetype( const std::string& name );

				bool TryGetArchetype( const std::string& name, Archetype& out_archetype ) const;

			private:
				bool IsArchetypeValid( const Archetype& archetype ) const;

				std::unordered_map< std::string, Archetype > _archetypes;
		};
	}
}
