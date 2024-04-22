#pragma once
#include <sstream>

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

	void Tick(float tickElapsedTime) override
	{
		std::stringstream ss;
		ss << tickElapsedTime;
		Common::LOG_INFO(ss.str());
		TransformComponent& transform = GetComponent<TransformComponent>();
		transform.posX += 50*tickElapsedTime;
	}
};
