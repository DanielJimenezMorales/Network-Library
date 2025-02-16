#pragma once
#include "Vec2f.h"
#include <stdint.h>

namespace ECS
{
	class GameEntity;
}

class InputState;
struct TransformComponent;
struct PlayerControllerComponent;

class PlayerSimulator
{
	public:
		static void Simulate( const InputState& inputs, ECS::GameEntity& playerEntity, float32 elapsedTime );

	private:
		static Vec2f UpdatePosition( const Vec2f& inputs, const TransformComponent& transform,
		                             const PlayerControllerComponent& configuration, float32 elapsedTime );
		static void ApplyPosition( const Vec2f& position, TransformComponent& transform );
		static Vec2f UpdateLookAt( const Vec2f& virtualMousePosition, const TransformComponent& transform,
		                           const PlayerControllerComponent& configuration, float32 elapsedTime );
};