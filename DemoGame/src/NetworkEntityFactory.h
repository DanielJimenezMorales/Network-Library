#pragma once
#include "Peer.h"
#include "INetworkEntityFactory.h"

class Scene;
class NetworkVariableChangesHandler;

class NetworkEntityFactory : public NetLib::INetworkEntityFactory
{
public:
	NetworkEntityFactory() : NetLib::INetworkEntityFactory(), _scene(nullptr), _peerType(NetLib::PeerType::None) {};
	void SetScene(Scene* scene);
	void SetPeerType(NetLib::PeerType peerType);
	int CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, uint32_t controlledByPeerId, float posX, float posY, NetLib::NetworkVariableChangesHandler* networkVariableChangeHandler) override;
	void DestroyNetworkEntityObject(uint32_t gameEntity) override;

private:
	Scene* _scene;

	NetLib::PeerType _peerType;
};
