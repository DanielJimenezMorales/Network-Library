#pragma once
#include "SDL.h"
#include "NetworkEntityFactory.h"
#include "IPreTickSystem.h"
#include "IPosTickSystem.h"

class Peer;
class Scene;
class IInputController;
class EntityContainer;

class NetworkSystem : public IPreTickSystem, public IPosTickSystem
{
public:
	NetworkSystem() {}
	void Initialize(SDL_Renderer* renderer, Scene* scene, NetLib::PeerType type, IInputController* inputController);
	void PreTick(EntityContainer& entityContainer, float elapsedTime) const override;
	void PosTick(EntityContainer& entityContainer, float elapsedTime) const override;
	void Release();

private:
	//TODO Move these to different components. ECS System shouldn't contain any data. Just logic
	NetLib::Peer* _networkPeer;
	NetworkEntityFactory _networkEntityFactory;
};
