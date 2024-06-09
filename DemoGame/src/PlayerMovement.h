#pragma once
#include <sstream>

#include "ScriptableSystem.h"
#include "TransformComponent.h"
#include "Logger.h"
#include "Vec2f.h"

class PlayerMovement : public ScriptableSystem
{
public:

protected:
	void Create() override
	{
		LOG_INFO("PLAYER MOVEMENT CREATE");
	}

	void Tick(float tickElapsedTime) override;

private:
	Vec2f UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, float elapsedTime) const;
	void ApplyPosition(const Vec2f& position, TransformComponent& transform);

	int _speed = 250;
};

class PlayerDummyMovement : public ScriptableSystem
{
public:

protected:
	void Create() override
	{
		LOG_INFO("PLAYER DUMMY MOVEMENT CREATE");
	}

	void Tick(float tickElapsedTime) override;
};
