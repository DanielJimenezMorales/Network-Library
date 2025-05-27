#pragma once
#include "ecs/world.h"

const uint32 FIXED_FRAMES_PER_SECOND = 50;
const float32 FIXED_FRAME_TARGET_DURATION = 1.0f / FIXED_FRAMES_PER_SECOND;

namespace Engine
{
	class IWorldInitializer;
}

class Game
{
	public:
		Game() = default;

		bool Init( Engine::IWorldInitializer* world_initializer );
		void GameLoop();
		bool Release();

	private:
		void HandleEvents( float32 tickElapsedTime );
		void PreTick( float32 tickElapsedTime );
		void Tick( float32 tickElapsedTime );
		void PosTick( float32 tickElapsedTime );
		void Update( float32 elapsedTime );
		void Render( float32 elapsed_time );
		void EndOfFrame();

		bool _isRunning;

		Engine::ECS::World _activeScene;
};