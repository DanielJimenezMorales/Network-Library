#include "Game.h"

#include "logger.h"

//TODO Remove these NetLib dependencies from the Engine
#include "core/peer.h"
#include "core/initializer.h"

#include "i_world_initializer.h"

namespace Engine
{
	ECS::World& Game::GetActiveWorld()
	{
		return _activeScene;
	}

	AssetManager& Game::GetAssetManager()
	{
		return _assetManager;
	}

	bool Game::Init( IWorldInitializer* world_initializer )
	{
		world_initializer->SetUpWorld( *this );

		_isRunning = true;
		return true;
	}

	void Game::GameLoop()
	{
		//TODO create the owns game engine clock
		NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
		float64 accumulator = 0.0;

		while ( _isRunning )
		{
			timeClock.UpdateLocalTime();
			accumulator += timeClock.GetElapsedTimeSeconds();

			HandleEvents( static_cast< float32 >( timeClock.GetElapsedTimeSeconds() ) );

			while ( accumulator >= FIXED_FRAME_TARGET_DURATION )
			{
				PreTick( FIXED_FRAME_TARGET_DURATION );
				Tick( FIXED_FRAME_TARGET_DURATION );
				PosTick( FIXED_FRAME_TARGET_DURATION );
				accumulator -= FIXED_FRAME_TARGET_DURATION;
			}

			Update( static_cast< float32 >( timeClock.GetElapsedTimeSeconds() ) );
			Render( static_cast< float32 >( timeClock.GetElapsedTimeSeconds() ) );
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
		//TODO This shouldn't be here
		NetLib::Initializer::Finalize();

		return true;
	}
} // namespace Engine
