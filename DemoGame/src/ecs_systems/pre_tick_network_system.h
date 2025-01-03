#pragma once
#include "ecs/i_simple_system.h"

class PreTickNetworkSystem : public ECS::ISimpleSystem
{
	public:
		PreTickNetworkSystem();

		void Execute( GameEntity& entity, float32 elapsed_time ) override;
};
