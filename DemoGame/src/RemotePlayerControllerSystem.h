#pragma once
#include "ITickSystem.h"

class RemotePlayerControllerSystem : public ITickSystem
{
	public:
		void Tick( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const;
};