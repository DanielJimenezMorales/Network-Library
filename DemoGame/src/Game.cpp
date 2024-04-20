#include <sstream>
#include <SDL_image.h>

#include "Game.h"
#include "Logger.h"
#include "Server.h"
#include "Client.h"
#include "Initializer.h"
#include "GameEntity.h"
#include "SpriteRendererComponent.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"
#include "PlayerMovement.h"

bool Game::Init()
{
    std::cout << "Select:\nServer: 0\nClient: 1\n";

    int clientOrServer;
    std::cin >> clientOrServer;
    NetLib::Initializer::Initialize();

    if (clientOrServer == 0)
    {
        _peer = new NetLib::Server(2);
    }
    else if (clientOrServer == 1)
    {
        _peer = new NetLib::Client(5);
    }

    if (!_peer->Start())
    {
        Common::LOG_ERROR("Peer startup failed");
    }

    int result = InitSDL();
    if (result != 0)
    {
        std::stringstream ss;
        ss << "Error while initializing SDL. Error code: " << SDL_GetError();
        Common::LOG_ERROR(ss.str());
        return false;
    }

    result = CreateWindowAndRenderer();
    if (result != 0)
    {
        std::stringstream ss;
        ss << "Error while initializing SDL window. Error code: " << SDL_GetError();
        Common::LOG_ERROR(ss.str());
        return false;
    }

    SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
    _isRunning = true;

    SDL_Surface* imageSurface = IMG_Load("sprites/PlayerSprites/playerHead.png");
    if (imageSurface == nullptr)
    {
        Common::LOG_INFO("HH");
    }

    SDL_Texture* imageTexture = SDL_CreateTextureFromSurface(_renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    SDL_Rect sourceTextureRect;
    result = SDL_QueryTexture(imageTexture, NULL, NULL, &sourceTextureRect.w, &sourceTextureRect.h);
    if (result == 0)
    {
        Common::LOG_INFO("COOL");
    }

    sourceTextureRect.x = 0;
    sourceTextureRect.y = 0;

    GameEntity playerEntity = _activeScene.CreateGameEntity();
    playerEntity.AddComponent<SpriteRendererComponent>(sourceTextureRect, imageTexture);
    TransformComponent& playerTransform = playerEntity.GetComponent<TransformComponent>();
    playerTransform.posX = 256;
    playerTransform.posY = 56;

    playerEntity.AddComponent<ScriptComponent>().Bind<PlayerMovement>();
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
            _activeScene.Update(FIXED_FRAME_TARGET_DURATION);
            _peer->Tick(FIXED_FRAME_TARGET_DURATION);

            accumulator -= FIXED_FRAME_TARGET_DURATION;
        }

        Render();
    }
}

void Game::HandleEvents()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_QUIT)
        {
            _isRunning = false;
        }
    }
}

void Game::Update()
{
}

void Game::Render()
{
    SDL_RenderClear(_renderer);

    _activeScene.Render(_renderer);

    SDL_RenderPresent(_renderer);
}

bool Game::Release()
{
    if (!_peer->Stop())
    {
        Common::LOG_ERROR("Peer stop failed");
    }

    delete _peer;
    _peer = nullptr;

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