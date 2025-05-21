#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct PlayerControllerComponentConfiguration : public Engine::ECS::ComponentConfiguration
{
		PlayerControllerComponentConfiguration();
		PlayerControllerComponentConfiguration( uint32 movement_speed, uint32 fire_rate_per_second );

		PlayerControllerComponentConfiguration* Clone() const override;

		uint32 movementSpeed;
		uint32 fireRatePerSecond;
};