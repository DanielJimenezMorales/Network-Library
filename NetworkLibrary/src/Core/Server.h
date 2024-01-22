#pragma once

#include <vector>

#include "Peer.h"
#include "RemotePeer.h"

class PendingConnection;
class ConnectionRequestMessage;
class ConnectionChallengeResponseMessage;

class Server : public Peer
{
public:
	Server(int maxConnections);
	~Server() override;

protected:
	bool StartConcrete() override;
	void ProcessMessage(const Message& message, const Address& address) override;
	void TickConcrete(float elapsedTime) override;
	void DisconnectRemotePeerConcrete(RemotePeer& remotePeer) override;
	bool StopConcrete() override;

private:
	uint64_t GenerateServerSalt() const;

	void ProcessConnectionRequest(const ConnectionRequestMessage& message, const Address& address);
	void ProcessConnectionChallengeResponse(const ConnectionChallengeResponseMessage& message, const Address& address);

	/// <summary>
	/// This method checks if a new client is able to connect to server
	/// </summary>
	/// <param name="address">The network address information of the client trying to connect</param>
	/// <returns>
	/// 0 = Is able to connect.
	/// 1 = Is already connected.
	/// -1 = Unable to connect, the server has reached its maximum connections.
	/// </returns>
	int IsClientAbleToConnect(const Address& address) const;
	void AddNewRemoteClient(int remoteClientSlotIndex, const Address& address, uint64_t dataPrefix);

	void CreateConnectionChallengeMessage(const Address& address, int pendingConnectionIndex);
	void CreateConnectionApprovedMessage(RemotePeer& remoteClient);
	void CreateDisconnectionMessage(RemotePeer& remoteClient);
	void SendConnectionDeniedPacket(const Address& address) const;
	void SendPacketToRemoteClient(const RemotePeer& remoteClient, const NetworkPacket& packet) const;

	unsigned int _nextAssignedRemoteClientID = 1;
};

