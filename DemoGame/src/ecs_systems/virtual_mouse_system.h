#pragma once
#include "ecs/i_simple_system.h"

class VirtualMouseSystem : public ECS::ISimpleSystem
{
	public:
		void Execute( ECS::World& world, float32 elapsed_time ) override;
};
