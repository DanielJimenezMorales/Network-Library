#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct PlayerControllerComponentConfiguration : public ECS::ComponentConfiguration
{
		PlayerControllerComponentConfiguration();
		PlayerControllerComponentConfiguration( uint32 movement_speed );

		PlayerControllerComponentConfiguration* Clone() const override;

		uint32 movementSpeed;
};