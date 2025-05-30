#pragma once
#include <string>

namespace Engine
{
	namespace ECS
	{
		struct ComponentConfiguration
		{
				ComponentConfiguration( const ComponentConfiguration& other ) = default;
				ComponentConfiguration( ComponentConfiguration&& other ) noexcept = default;

				virtual ~ComponentConfiguration() {}

				ComponentConfiguration& operator=( const ComponentConfiguration& other ) = default;
				ComponentConfiguration& operator=( ComponentConfiguration&& other ) noexcept = default;

				virtual ComponentConfiguration* Clone() const = 0;

				std::string name;

			protected:
				ComponentConfiguration( const std::string& name );
		};
	}
}
