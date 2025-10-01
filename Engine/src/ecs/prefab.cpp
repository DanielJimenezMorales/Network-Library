#include "prefab.h"

namespace Engine
{
	namespace ECS
	{
		Prefab::Prefab()
		    : name()
		    , archetype()
		    , componentConfigurations()
		    , childrenPrefabs()
		{
		}

		Prefab::Prefab( const Prefab& other )
		    : name( other.name )
		    , archetype( other.archetype )
		    , childrenPrefabs( other.childrenPrefabs )
		    , componentConfigurations()
		{
			CopyComponents( other.componentConfigurations );
		}

		Prefab::~Prefab()
		{
			Free();
		}

		Prefab& ECS::Prefab::operator=( const Prefab& other )
		{
			if ( this == &other )
			{
				return *this;
			}

			Free();
			name = other.name;
			archetype = other.archetype;
			childrenPrefabs = other.childrenPrefabs;
			CopyComponents( other.componentConfigurations );
			return *this;
		}

		void Prefab::Free()
		{
			auto it = componentConfigurations.begin();
			for ( ; it != componentConfigurations.end(); ++it )
			{
				if ( it->second != nullptr )
				{
					delete it->second;
				}
			}
		}

		void Prefab::CopyComponents( const std::unordered_map< std::string, ComponentConfiguration* >& other )
		{
			componentConfigurations.reserve( other.size() );
			for ( auto cit = other.cbegin(); cit != other.cend(); ++cit )
			{
				componentConfigurations[ cit->first ] = cit->second->Clone();
			}
		}
	} // namespace ECS
} // namespace Engine
