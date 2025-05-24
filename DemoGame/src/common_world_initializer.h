#pragma once
#include "i_world_initializer.h"

namespace Engine
{
	namespace ECS
	{
		class GameEntity;
		class Prefab;
	}
}

class CommonWorldInitializer : public Engine::IWorldInitializer
{
	public:
		CommonWorldInitializer();

		void SetUpWorld( Engine::ECS::World& world ) override;

	private:
		// TODO Temp method. It shouldn't go here
		void ConfigureCameraComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab ) const;
		void ConfigureHealthComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab ) const;
};
