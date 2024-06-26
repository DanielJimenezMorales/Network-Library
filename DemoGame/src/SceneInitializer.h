#pragma once
#include "Peer.h"

class Scene;
class InputHandler;
class TextureLoader;

class SceneInitializer
{
public:
	void InitializeScene(Scene& scene, NetLib::PeerType networkPeerType, TextureLoader* textureLoader, InputHandler& inputHandler) const;
};