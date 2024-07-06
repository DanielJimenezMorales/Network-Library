#pragma once
#include "ITickSystem.h"
#include <cstdint>

class GameEntity;
struct Vec2f;
struct TransformComponent;
struct PlayerControllerConfiguration;
struct InputComponent;

class PlayerControllerSystem : public ITickSystem
{
public:
	PlayerControllerSystem() : ITickSystem()
	{
	}
	
	void Tick(EntityContainer& entityContainer, float elapsedTime) const;

private:
	void TickPlayerController(GameEntity& playerEntity, const InputComponent& inputComponent, float elapsedTime) const;
	Vec2f UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, const PlayerControllerConfiguration& configuration, float elapsedTime) const;
	void ApplyPosition(const Vec2f& position, TransformComponent& transform) const;
};
