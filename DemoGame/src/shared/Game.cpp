#include "Game.h"

#include "logger.h"

#include "core/peer.h"
#include "core/initializer.h"

#include "i_world_initializer.h"

bool Game::Init( Engine::IWorldInitializer* world_initializer )
{
	world_initializer->SetUpWorld(_activeScene);

	_isRunning = true;
	return true;
}

void Game::GameLoop()
{
	NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
	float64 accumulator = 0.0;

	while ( _isRunning )
	{
		timeClock.UpdateLocalTime();
		accumulator += timeClock.GetElapsedTimeSeconds();

		HandleEvents( timeClock.GetElapsedTimeSeconds() );

		while ( accumulator >= FIXED_FRAME_TARGET_DURATION )
		{
			PreTick( FIXED_FRAME_TARGET_DURATION );
			Tick( FIXED_FRAME_TARGET_DURATION );
			PosTick( FIXED_FRAME_TARGET_DURATION );
			accumulator -= FIXED_FRAME_TARGET_DURATION;
		}

		Update( timeClock.GetElapsedTimeSeconds() );
		Render( timeClock.GetElapsedTimeSeconds() );
		EndOfFrame();
	}
}

void Game::HandleEvents( float32 tickElapsedTime )
{
	_activeScene.InputHandling( tickElapsedTime );
}

void Game::PreTick( float32 tickElapsedTime )
{
	_activeScene.PreTick( tickElapsedTime );
}

void Game::Tick( float32 tickElapsedTime )
{
	_activeScene.Tick( tickElapsedTime );
}

void Game::PosTick( float32 tickElapsedTime )
{
	_activeScene.PosTick( tickElapsedTime );
}

void Game::Update( float32 elapsedTime )
{
	_activeScene.Update( elapsedTime );
}

void Game::Render( float32 elapsed_time )
{
	_activeScene.Render( elapsed_time );
}

void Game::EndOfFrame()
{
	_activeScene.EndOfFrame();
}

bool Game::Release()
{
	NetLib::Initializer::Finalize();

	return true;
}
