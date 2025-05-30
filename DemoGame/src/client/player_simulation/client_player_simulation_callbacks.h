#pragma once

namespace Engine
{
	namespace ECS
	{
		class World;
		class GameEntity;
	}
}

void OnShotPerformed( Engine::ECS::World& world, const Engine::ECS::GameEntity& player_entity );
