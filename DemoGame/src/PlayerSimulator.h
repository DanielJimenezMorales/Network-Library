#pragma once
#include "Vec2f.h"
#include <stdint.h>

class InputState;
class GameEntity;
struct TransformComponent;
struct PlayerControllerConfiguration;

class PlayerSimulator
{
public:
	static void Simulate(const InputState& inputs, GameEntity& playerEntity, float elapsedTime);

private:
	static Vec2f UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, const PlayerControllerConfiguration& configuration, float elapsedTime);
	static void ApplyPosition(const Vec2f& position, TransformComponent& transform);
	static Vec2f UpdateLookAt(int32_t mouseDeltaX, int32_t mouseDeltaY, const TransformComponent& transform, const PlayerControllerConfiguration& configuration, float elapsedTime);
};