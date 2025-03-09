#pragma once
#include "ecs/i_simple_system.h"

namespace ECS
{
	class World;
}

class ClientPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ClientPlayerControllerSystem( ECS::World* world );

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

	private:
		ECS::World* _world;
};
