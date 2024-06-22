#pragma once
#include "SDL.h"
#include "Peer.h"
#include "INetworkEntityFactory.h"

class IInputController;
class Scene;
class NetworkVariableChangesHandler;

class NetworkEntityFactory : public NetLib::INetworkEntityFactory
{
public:
	NetworkEntityFactory() : NetLib::INetworkEntityFactory(), _scene(nullptr), _renderer(nullptr), _inputController(nullptr), _peerType(NetLib::PeerType::None) {};
	void SetRenderer(SDL_Renderer* renderer);
	void SetScene(Scene* scene);
	void SetKeyboard(IInputController* inputController);
	void SetPeerType(NetLib::PeerType peerType);
	int CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, float posX, float posY, NetLib::NetworkVariableChangesHandler* networkVariableChangeHandler) override;
	void DestroyNetworkEntityObject(uint32_t gameEntity) override;

private:
	Scene* _scene;
	SDL_Renderer* _renderer;
	IInputController* _inputController;

	NetLib::PeerType _peerType;
};
