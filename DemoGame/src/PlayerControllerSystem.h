#pragma once
#include "ITickSystem.h"
#include <cstdint>

class GameEntity;
struct Vec2f;
struct TransformComponent;
struct PlayerControllerConfiguration;
struct InputComponent;
class InputState;

class PlayerControllerSystem : public ITickSystem
{
	public:
		PlayerControllerSystem()
		    : ITickSystem()
		{
		}

		void Tick( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const;

	private:
		void ProcessInputs( ECS::EntityContainer& entityContainer, InputState& outInputState ) const;
		void SendInputsToServer( ECS::EntityContainer& entityContainer, const InputState& inputState ) const;
};
