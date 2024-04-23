#pragma once
#include <SDL.h>

#include "Peer.h"
#include "Scene.h"
#include "InputHandler.h"

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
	void Update(float elapsedTime);
	void Tick(float tickElapsedTime);
	void Render();

	SDL_Window* _window;
	SDL_Renderer* _renderer;
	bool _isRunning;

	NetLib::Peer* _peer;
	Scene _activeScene;
	InputHandler _inputHandler;
};