#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct PlayerControllerConfiguration : public ECS::ComponentConfiguration
{
	PlayerControllerConfiguration();
		PlayerControllerConfiguration( uint32 movement_speed );

		PlayerControllerConfiguration* Clone() const override;

		uint32 movementSpeed;
};