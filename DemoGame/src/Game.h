#pragma once
#include <SDL.h>

#include "Scene.h"
#include "InputHandler.h"
#include "NetworkSystem.h"

const unsigned int FIXED_FRAMES_PER_SECOND = 50;
const float FIXED_FRAME_TARGET_DURATION = 1.0f / FIXED_FRAMES_PER_SECOND;

class Game
{
public:
	Game() = default;

	bool Init();
	void GameLoop();
	bool Release();

private:
	int InitSDL();
	int CreateWindowAndRenderer();

	void HandleEvents();
	void PreTick();
	void Tick(float tickElapsedTime);
	void PosTick();
	void Update(float elapsedTime);
	void Render();

	SDL_Window* _window;
	SDL_Renderer* _renderer;
	bool _isRunning;

	Scene _activeScene;
	InputHandler _inputHandler;
	NetworkSystem _networkSystem;
};