#pragma once
#include "ecs/i_simple_system.h"

namespace Engine
{
	class RenderClearSystem : public ECS::ISimpleSystem
	{
		public:
			RenderClearSystem();

			void Execute( ECS::World& world, float32 elapsed_time ) override;
	};
}
