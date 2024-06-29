#pragma once
#include "ITickSystem.h"
#include <cstdint>

class GameEntity;
struct Vec2f;
struct TransformComponent;

//TODO Add this within PlayerControllerComponent
struct PlayerControllerConfiguration
{
	uint32_t movementSpeed = 250;
};

class PlayerControllerSystem : public ITickSystem
{
public:
	PlayerControllerSystem() : ITickSystem()
	{
	}
	
	void Tick(EntityContainer& entityContainer, float elapsedTime) const;

private:
	void TickPlayerController(GameEntity& playerEntity, float elapsedTime) const;
	Vec2f UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, float elapsedTime) const;
	void ApplyPosition(const Vec2f& position, TransformComponent& transform) const;
};
