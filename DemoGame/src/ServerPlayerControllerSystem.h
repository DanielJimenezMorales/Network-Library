#pragma once
#include "ITickSystem.h"
#include <cstdint>

class GameEntity;
class InputState;

class ServerPlayerControllerSystem : public ITickSystem
{
	public:
		void Tick( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const;
};