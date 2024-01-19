#pragma once

#include <vector>

#include "Peer.h"
#include "RemoteClient.h"

class PendingConnection;
class ConnectionRequestMessage;
class ConnectionChallengeResponseMessage;

#define REMOTE_CLIENT_INACTIVITY_TIME 5.0f

class Server : public Peer
{
public:
	Server(int maxConnections);
	~Server() override;


private:
	bool StartConcrete() override;
	void ProcessMessage(const Message& message, const Address& address) override;
	void TickConcrete(float elapsedTime) override;
	bool StopConcrete() override;

private:
	uint64_t GenerateServerSalt() const;

	void ProcessConnectionRequest(const ConnectionRequestMessage& message, const Address& address);
	void ProcessConnectionChallengeResponse(const ConnectionChallengeResponseMessage& message, const Address& address);

	void HandleConnectedClientsInactivity(float elapsedTime);

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
	int GetEmptyClientSlot() const;
	bool IsClientAlreadyConnected(const Address& address) const;
	void AddNewRemoteClient(int remoteClientSlotIndex, const Address& address, uint64_t dataPrefix);
	int FindExistingClientIndex(const Address& address) const;

	void SendData();
	void CreateConnectionChallengeMessage(const Address& address, int pendingConnectionIndex);
	void CreateConnectionApprovedMessage(RemoteClient& remoteClient);
	void CreateDisconnectionMessage(RemoteClient& remoteClient);
	void SendConnectionDeniedPacket(const Address& address) const;
	void SendPacketToRemoteClient(const RemoteClient& remoteClient, const NetworkPacket& packet) const;

	void DisconnectRemoteClient(unsigned int index);
	void FinishRemoteClientsDisconnection();

	int _maxConnections;
	std::vector<bool> _remoteClientSlots;
	std::vector<RemoteClient> _remoteClients;
	std::queue<unsigned int> _remoteClientSlotIDsToDisconnect;

	std::vector<PendingConnection> _pendingConnections;

	unsigned int _nextAssignedRemoteClientID = 1;
};

