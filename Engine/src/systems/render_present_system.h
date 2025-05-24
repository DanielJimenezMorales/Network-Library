#pragma once
#include "ecs/i_simple_system.h"

namespace Engine
{
	class RenderPresentSystem : public ECS::ISimpleSystem
	{
		public:
			RenderPresentSystem();

			void Execute( ECS::World& world, float32 elapsed_time ) override;
	};
}
