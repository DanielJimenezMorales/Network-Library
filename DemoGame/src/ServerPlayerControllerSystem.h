#pragma once
#include "ITickSystem.h"
#include <cstdint>

class GameEntity;
struct Vec2f;
struct TransformComponent;
struct PlayerControllerConfiguration;
class InputState;

class ServerPlayerControllerSystem : public ITickSystem
{
public:
	void Tick(EntityContainer& entityContainer, float elapsedTime) const;

private:
	void TickPlayerController(GameEntity& playerEntity, const InputState& inputState, float elapsedTime) const;
	Vec2f UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, const PlayerControllerConfiguration& configuration, float elapsedTime) const;
	void ApplyPosition(const Vec2f& position, TransformComponent& transform) const;
};