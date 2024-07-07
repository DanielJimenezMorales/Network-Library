#include "ServerOnRemotePeerConnectDisconnectCallbacks.h"
#include "Scene.h"
#include "NetworkPeerComponent.h"
#include "GameEntity.hpp"

void ServerOnRemotePeerConnectDisconnectCallbacks::OnRemotePeerConnect() const
{
	GameEntity networkPeerEntity = _scene->GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();
	serverPeer->CreateNetworkEntity(10, 5, 256.f, 256.f);
}
