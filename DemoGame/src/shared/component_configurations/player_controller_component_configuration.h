#pragma once
#include "numeric_types.h"

#include "ecs/component_configuration.h"

struct PlayerControllerComponentConfiguration : public Engine::ECS::ComponentConfiguration
{
		// TODO I don't like having a component config with no data members
		PlayerControllerComponentConfiguration();

		PlayerControllerComponentConfiguration* Clone() const override;
};