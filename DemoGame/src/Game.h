#pragma once
#include <SDL.h>

#include "Scene.h"
#include "InputHandler.h"
#include "TextureLoader.h"

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
	void PreTick(float tickElapsedTime);
	void Tick(float tickElapsedTime);
	void PosTick(float tickElapsedTime);
	void Update(float elapsedTime);
	void Render();

	SDL_Window* _window;
	SDL_Renderer* _renderer;
	bool _isRunning;

	Scene _activeScene;
	//TODO The input handler should also be witin an ECS component
	InputHandler _inputHandler;
	TextureLoader _textureLoader;
};