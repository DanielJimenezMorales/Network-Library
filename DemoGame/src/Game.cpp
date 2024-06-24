#include "Game.h"
#include "Logger.h"
#include "Peer.h"
#include "Initializer.h"
#include "SceneInitializer.h"

bool Game::Init()
{
    LOG_INFO("Select:");
    LOG_INFO("Server: 0");
    LOG_INFO("Client: 1");

    int clientOrServer;
    std::cin >> clientOrServer;

    int result = InitSDL();
    if (result != 0)
    {
        LOG_ERROR("Error while initializing SDL. Error code: %s", SDL_GetError());
        return false;
    }

    result = CreateWindowAndRenderer();
    if (result != 0)
    {
        LOG_ERROR("Error while initializing SDL window. Error code: %s", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
    _isRunning = true;

    SceneInitializer sceneInitializer;

    NetLib::PeerType peerType = clientOrServer == 0 ? NetLib::PeerType::ServerMode : NetLib::PeerType::ClientMode;
    sceneInitializer.InitializeScene(_activeScene, peerType, _renderer, _inputHandler);

    return true;
}

void Game::GameLoop()
{
    NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
    double accumulator = 0.0;

    while (_isRunning)
    {
        timeClock.UpdateLocalTime();
        accumulator += timeClock.GetElapsedTimeSeconds();

        HandleEvents();

        while (accumulator >= FIXED_FRAME_TARGET_DURATION)
        {
            PreTick(FIXED_FRAME_TARGET_DURATION);
            Tick(FIXED_FRAME_TARGET_DURATION);
            PosTick(FIXED_FRAME_TARGET_DURATION);
            accumulator -= FIXED_FRAME_TARGET_DURATION;
        }
        
        Update(timeClock.GetElapsedTimeSeconds());

        Render();
    }
}

void Game::HandleEvents()
{
    _inputHandler.PreHandleEvents();

    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_QUIT)
        {
            _isRunning = false;
        }
        else
        {
            _inputHandler.HandleEvent(ev);
        }
    }

    _inputHandler.PostHandleEvents();
}

void Game::PreTick(float tickElapsedTime)
{
    _activeScene.PreTick(tickElapsedTime);
}

void Game::Tick(float tickElapsedTime)
{
    _activeScene.Tick(tickElapsedTime);
}

void Game::PosTick(float tickElapsedTime)
{
    _activeScene.PosTick(tickElapsedTime);
}

void Game::Update(float elapsedTime)
{
    _activeScene.Update(elapsedTime);
}

void Game::Render()
{
    SDL_RenderClear(_renderer);

    _activeScene.Render(_renderer);

    SDL_RenderPresent(_renderer);
}

bool Game::Release()
{
    NetLib::Initializer::Finalize();

    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();

    return true;
}

int Game::InitSDL()
{
    return SDL_Init(SDL_INIT_EVERYTHING);
}

int Game::CreateWindowAndRenderer()
{
    return SDL_CreateWindowAndRenderer(512, 512, SDL_WINDOW_SHOWN, &_window, &_renderer);
}
