#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>

#include "RemoteClient.h"

class Buffer;
class Address;
class PendingConnection;
class ConnectionRequestMessage;
class ConnectionChallengeResponseMessage;

#define REMOTE_CLIENT_INACTIVITY_TIME 5.0f

class Server
{
public:
	Server(int maxConnections);
	~Server();

	bool Start();
	int Stop();

	void Tick(float elapsedTime);

private:
	bool InitSocket();

	uint64_t GenerateServerSalt() const;

	bool IsThereNewDataToProcess() const;
	void ProcessReceivedData();
	void ProcessDatagram(Buffer& buffer, const Address& address);
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
	void SendDisconnectionPacketToRemoteClient(const RemoteClient& remoteClient) const;
	void SendConnectionChallengePacket(const Address& address, int pendingConnectionIndex) const;
	void SendConnectionDeniedPacket(const Address& address) const;
	void SendConnectionApprovedPacketToRemoteClient(const RemoteClient& remoteClient) const;
	void SendDatagramToRemoteClient(const RemoteClient& remoteClient, const Buffer& buffer) const;
	void SendDataToAddress(const Buffer& buffer, const Address& address) const;

	void DisconnectRemoteClient(unsigned int index);

	int _maxConnections;
	std::vector<bool> _remoteClientSlots;
	std::vector<RemoteClient*> _remoteClients;

	std::vector<PendingConnection> _pendingConnections;
	SOCKET _listenSocket = INVALID_SOCKET;

	unsigned int _nextAssignedRemoteClientIndex = 1;
};

