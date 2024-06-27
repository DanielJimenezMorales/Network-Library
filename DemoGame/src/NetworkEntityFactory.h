#pragma once
#include "Peer.h"
#include "INetworkEntityFactory.h"

class IInputController;
class Scene;
class TextureLoader;
class NetworkVariableChangesHandler;

class NetworkEntityFactory : public NetLib::INetworkEntityFactory
{
public:
	NetworkEntityFactory() : NetLib::INetworkEntityFactory(), _scene(nullptr), _textureLoader(nullptr), _inputController(nullptr), _peerType(NetLib::PeerType::None) {};
	void SetTextureLoader(TextureLoader* textureLoader);
	void SetScene(Scene* scene);
	void SetKeyboard(IInputController* inputController);
	void SetPeerType(NetLib::PeerType peerType);
	int CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, float posX, float posY, NetLib::NetworkVariableChangesHandler* networkVariableChangeHandler) override;
	void DestroyNetworkEntityObject(uint32_t gameEntity) override;

private:
	Scene* _scene;
	IInputController* _inputController;
	TextureLoader* _textureLoader;

	NetLib::PeerType _peerType;
};
