#pragma once
#include "IUpdateSystem.h"

class CrosshairFollowMouseSystem : public IUpdateSystem
{
	public:
		void Update( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const override;
};