#pragma once
#include "ecs/i_simple_system.h"

class ClientLocalPlayerServerReconciliatorSystem : public ECS::ISimpleSystem
{
	public:
		ClientLocalPlayerServerReconciliatorSystem();

		void Execute( ECS::World& world, float32 elapsed_time ) override;
};
