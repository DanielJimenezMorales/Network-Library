#pragma once
#include "ecs/i_simple_system.h"

class InterpolatedPlayerObjectUpdaterSystem : public ECS::ISimpleSystem
{
public:
	InterpolatedPlayerObjectUpdaterSystem();

	void Execute(ECS::World& world, float32 elapsed_time) override;
};
