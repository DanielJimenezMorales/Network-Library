#pragma once
#include "Peer.h"
#include "INetworkEntityFactory.h"

class Scene;
class NetworkVariableChangesHandler;

/// <summary>
/// Player network entities factory. IMPORTANT: Players will be using network variable ids from 1 to 100.
/// </summary>
class NetworkEntityFactory : public NetLib::INetworkEntityFactory
{
public:
	NetworkEntityFactory() : NetLib::INetworkEntityFactory(), _scene(nullptr), _peerType(NetLib::PeerType::None) {}
	~NetworkEntityFactory() {}

	void SetScene(Scene* scene);
	void SetPeerType(NetLib::PeerType peerType);
	int CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, uint32_t controlledByPeerId, float posX, float posY, NetLib::NetworkVariableChangesHandler* networkVariableChangeHandler) override;
	void DestroyNetworkEntityObject(uint32_t gameEntity) override;

private:
	Scene* _scene;

	NetLib::PeerType _peerType;
};
