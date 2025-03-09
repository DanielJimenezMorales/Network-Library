#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct TemporaryLifetimeComponentConfiguration : public ECS::ComponentConfiguration
{
		TemporaryLifetimeComponentConfiguration( float32 lifetime )
		    : ECS::ComponentConfiguration( "TemporaryLifetime" )
		    , lifetime( lifetime )
		{
		}

		TemporaryLifetimeComponentConfiguration* Clone() const override
		{
			return new TemporaryLifetimeComponentConfiguration( *this );
		}

		float32 lifetime;
};
