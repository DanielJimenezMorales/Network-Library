#pragma once
#include "ecs/i_simple_system.h"

namespace Engine
{
	class InputHandlerSystem : public ECS::ISimpleSystem
	{
		public:
			InputHandlerSystem();

			void Execute( ECS::World& world, float32 elapsed_time ) override;

		private:
			bool _isGameRunning;
	};
}