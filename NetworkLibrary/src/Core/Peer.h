#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <vector>
#include <queue>

#include "Address.h"
#include "PendingConnection.h"
#include "Socket.h"

class Buffer;
class Message;
class NetworkPacket;
class RemotePeer;

#define REMOTE_CLIENT_INACTIVITY_TIME 5.0f

enum PeerType : uint8_t
{
	None = 0,
	ClientMode = 1,
	ServerMode = 2
};

class Peer
{
public:
	bool Start();
	bool Tick(float elapsedTime);
	bool Stop();

	virtual ~Peer();

protected:
	Peer(PeerType type, int maxConnections);
	virtual bool StartConcrete() = 0;
	virtual void ProcessMessage(const Message& message, const Address& address) = 0;
	virtual void TickConcrete(float elapsedTime) = 0;
	virtual bool StopConcrete() = 0;

	void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;
	bool AddRemoteClient(const Address& addressInfo, uint16_t id, uint64_t dataPrefix);
	int FindFreeRemoteClientSlot() const;
	RemotePeer* GetRemoteClientFromAddress(const Address& address);
	bool IsRemotePeerAlreadyConnected(const Address& address) const;
	PendingConnection* GetPendingConnectionFromAddress(const Address& address);
	bool IsPendingConnectionAlreadyAdded(const Address& address) const;
	void RemovePendingConnection(const Address& address);

	std::queue<unsigned int> _remoteClientSlotIDsToDisconnect;
	std::vector<PendingConnection> _pendingConnections;
	const int _maxConnections;
	std::vector<bool> _remoteClientSlots;
	std::vector<RemotePeer> _remoteClients;

private:
	bool InitializeSocketsLibrary();
	bool BindSocket();

	void ProcessReceivedData();
	bool GetDatagramFromAddress(Buffer** buffer, Address* address);
	void ProcessDatagram(Buffer& buffer, const Address& address);

	int GetPendingConnectionIndexFromAddress(const Address& address) const;

	void SendData();
	void SendPacketToRemoteClient(const RemotePeer& remoteClient, const NetworkPacket& packet) const;
	void SendDataToAddress(const Buffer& buffer, const Address& address) const;

	void FinishRemoteClientsDisconnection();

	PeerType _type;
	Address _address;
	Socket _socket;

};
