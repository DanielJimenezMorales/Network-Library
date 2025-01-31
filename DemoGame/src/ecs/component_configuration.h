#pragma once
#include <string>

namespace ECS
{
	struct ComponentConfiguration
	{
			ComponentConfiguration( const std::string& name );
			ComponentConfiguration( const ComponentConfiguration& other ) = default;
			ComponentConfiguration( ComponentConfiguration&& other ) noexcept = default;

			virtual ~ComponentConfiguration() {}

			ComponentConfiguration& operator=( const ComponentConfiguration& other ) = default;
			ComponentConfiguration& operator=( ComponentConfiguration&& other ) noexcept = default;

			std::string name;
	};
}
