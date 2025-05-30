#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct TemporaryLifetimeComponentConfiguration : public Engine::ECS::ComponentConfiguration
{
		TemporaryLifetimeComponentConfiguration( float32 lifetime )
		    : Engine::ECS::ComponentConfiguration( "TemporaryLifetime" )
		    , lifetime( lifetime )
		{
		}

		TemporaryLifetimeComponentConfiguration* Clone() const override
		{
			return new TemporaryLifetimeComponentConfiguration( *this );
		}

		float32 lifetime;
};
