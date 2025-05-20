#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct HealthComponentConfiguration : public ECS::ComponentConfiguration
{
		HealthComponentConfiguration( uint32 max_health, uint32 current_health )
		    : ECS::ComponentConfiguration( "Health" )
		    , maxHealth( max_health )
		    , currentHealth( current_health )
		{
		}

		HealthComponentConfiguration* Clone() const override { return new HealthComponentConfiguration( *this ); }

		uint32 maxHealth;
		uint32 currentHealth;
};
