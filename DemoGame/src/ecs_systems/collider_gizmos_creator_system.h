#pragma once
#include "ecs/i_simple_system.h"

class ColliderGizmosCreatorSystem : public ECS::ISimpleSystem
{
	public:
		ColliderGizmosCreatorSystem();

		void Execute( GameEntity& entity, float32 elapsed_time ) override;
};
