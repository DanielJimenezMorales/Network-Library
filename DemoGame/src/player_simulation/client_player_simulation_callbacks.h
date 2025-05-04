#pragma once

namespace ECS
{
	class World;
	class GameEntity;
}

void OnShotPerformed( ECS::World& world, const ECS::GameEntity& player_entity );
