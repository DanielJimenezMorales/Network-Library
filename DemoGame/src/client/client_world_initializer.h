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

class ClientWorldInitializer : public Engine::IWorldInitializer
{
	public:
		ClientWorldInitializer();

		void SetUpWorld( Engine::Game& game ) override;

	private:
		// TODO Temp method. It shouldn't go here
		void ConfigureCameraComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab ) const;
		void ConfigureHealthComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab ) const;
};
