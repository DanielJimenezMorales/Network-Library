#pragma once
#include "ScriptableSystem.h"
#include "TransformComponent.h"
#include "Logger.h"

class PlayerMovement : public ScriptableSystem
{
protected:
	void Create() override
	{
		Common::LOG_INFO("PLAYER MOVEMENT CREATE");
	}

	void Update(float elapsedTime) override
	{
		TransformComponent& transform = GetComponent<TransformComponent>();
		++transform.posX;
	}
};
