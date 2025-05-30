#pragma once
#include <string>
#include <unordered_map>

#include "ecs/component_configuration.h"

namespace Engine
{
	namespace ECS
	{
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

			private:
				void Free();
				void CopyComponents( const std::unordered_map< std::string, ComponentConfiguration* >& other );
		};
	} // namespace ECS
}
