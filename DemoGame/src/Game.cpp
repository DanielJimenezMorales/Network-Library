#include "Game.h"

#include <SDL.h>

#include "logger.h"

#include "core/peer.h"
#include "core/initializer.h"

#include "SceneInitializer.h"

bool Game::Init()
{
	LOG_INFO( "Select:" );
	LOG_INFO( "Server: 0" );
	LOG_INFO( "Client: 1" );

	int32 clientOrServer;
	std::cin >> clientOrServer;

	SDL_InitSubSystem( SDL_INIT_EVENTS );

	_isRunning = true;

	SceneInitializer sceneInitializer;

	NetLib::PeerType peerType = clientOrServer == 0 ? NetLib::PeerType::SERVER : NetLib::PeerType::CLIENT;
	sceneInitializer.InitializeScene( _activeScene, peerType, _inputHandler );

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
	_inputHandler.PreHandleEvents();

	SDL_Event ev;
	while ( SDL_PollEvent( &ev ) )
	{
		if ( ev.type == SDL_QUIT )
		{
			_isRunning = false;
		}
		else
		{
			_inputHandler.HandleEvent( ev );
		}
	}

	_inputHandler.PostHandleEvents();

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
	SDL_Quit();

	return true;
}
