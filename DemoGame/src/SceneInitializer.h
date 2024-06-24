#pragma once
#include "Peer.h"
#include <SDL.h>

class Scene;
class InputHandler;

class SceneInitializer
{
public:
	void InitializeScene(Scene& scene, NetLib::PeerType networkPeerType, SDL_Renderer* renderer, InputHandler& inputHandler) const;
};