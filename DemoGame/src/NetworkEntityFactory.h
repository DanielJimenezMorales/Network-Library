#pragma once
#include "core/peer.h"

#include "replication/i_network_entity_factory.h"

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
	int32 CreateNetworkEntityObject(uint32 networkEntityType, uint32 networkEntityId, uint32 controlledByPeerId, float32 posX, float32 posY, NetLib::NetworkVariableChangesHandler* networkVariableChangeHandler) override;
	void DestroyNetworkEntityObject(uint32 gameEntity) override;

private:
	Scene* _scene;

	NetLib::PeerType _peerType;
};
