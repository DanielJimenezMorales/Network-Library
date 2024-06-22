#include <SDL_image.h>

#include "Game.h"
#include "Logger.h"
#include "TimeClock.h"
#include "GameEntity.hpp"
#include "SpriteRendererComponent.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"
#include "PlayerMovement.h"
#include "KeyboardController.h"
#include "InputComponent.h"
#include "InputActionIdsConfiguration.h"
#include "ScriptSystem.h"
#include "NetworkSystem.h"
#include "CurrentTickComponent.h"

KeyboardController* keyboard;

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

    //TEMP
    keyboard = new KeyboardController();
    InputButton button(JUMP_BUTTON, SDLK_q);
    keyboard->AddButtonMap(button);
    InputAxis axis(HORIZONTAL_AXIS, SDLK_d, SDLK_a);
    keyboard->AddAxisMap(axis);
    InputAxis axis2(VERTICAL_AXIS, SDLK_s, SDLK_w);
    keyboard->AddAxisMap(axis2);
    _inputHandler.AddController(keyboard);

    SDL_Surface* imageSurface = IMG_Load("sprites/PlayerSprites/playerHead.png");
    if (imageSurface == nullptr)
    {
        LOG_INFO("HH");
    }

    SDL_Texture* imageTexture = SDL_CreateTextureFromSurface(_renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    SDL_Rect sourceTextureRect;
    result = SDL_QueryTexture(imageTexture, NULL, NULL, &sourceTextureRect.w, &sourceTextureRect.h);
    if (result == 0)
    {
        LOG_INFO("COOL");
    }

    sourceTextureRect.x = 0;
    sourceTextureRect.y = 0;

    GameEntity currentTickEntity = _activeScene.CreateGameEntity();
    currentTickEntity.AddComponent<CurrentTickComponent>();

    ScriptSystem* scriptSystem = new ScriptSystem();
    _activeScene.AddUpdateSystem(scriptSystem);
    _activeScene.AddTickSystem(scriptSystem);
    NetworkSystem* networkSystem = new NetworkSystem();

    if (clientOrServer == 0)
    {
        networkSystem->Initialize(_renderer, &_activeScene, NetLib::PeerType::ServerMode, keyboard);
    }
    else if (clientOrServer == 1)
    {
        networkSystem->Initialize(_renderer, &_activeScene, NetLib::PeerType::ClientMode, keyboard);
    }

    _activeScene.AddPreTickSystem(networkSystem);
    _activeScene.AddPosTickSystem(networkSystem);

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
    //_networkSystem.Release();

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
