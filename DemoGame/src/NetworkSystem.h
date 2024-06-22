#pragma once
#include "SDL.h"
#include "NetworkEntityFactory.h"

class Peer;
class Scene;
class IInputController;

class NetworkSystem
{
public:
	NetworkSystem() : _currentTick(0) {}
	void Initialize(SDL_Renderer* renderer, Scene* scene, NetLib::PeerType type, IInputController* inputController);
	void PreTick();
	void Tick(float elapsedTime);
	void Release();

private:
	NetLib::Peer* _networkPeer;
	NetworkEntityFactory _networkEntityFactory;
	
	//TEMP
	int _currentTick;
};
