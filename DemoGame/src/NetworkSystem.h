#pragma once
#include "Peer.h"
#include "NetworkEntityFactory.h"
#include "Scene.h"
#include "SDL.h"

class NetworkSystem
{
public:
	NetworkSystem() : _currentTick(0) {}
	void Initialize(SDL_Renderer* renderer, Scene* scene, NetLib::PeerType type);
	void PreTick();
	void Tick(float elapsedTime);
	void Release();

private:
	NetLib::Peer* _networkPeer;
	NetworkEntityFactory _networkEntityFactory;
	
	//TEMP
	int _currentTick;
};
