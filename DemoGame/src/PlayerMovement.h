#pragma once
#include <sstream>

#include "ScriptableSystem.h"
#include "TransformComponent.h"
#include "Logger.h"
#include "Vec2f.h"

const int HORIZONTAL_AXIS = 1;
const int VERTICAL_AXIS = 2;

class PlayerMovement : public ScriptableSystem
{
protected:
	void Create() override
	{
		Common::LOG_INFO("PLAYER MOVEMENT CREATE");
	}

	void Tick(float tickElapsedTime) override;

private:
	Vec2f UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, float elapsedTime) const;
	void ApplyPosition(const Vec2f& position, TransformComponent& transform);

	int _speed = 250;
};
