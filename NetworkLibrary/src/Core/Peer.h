#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <vector>
#include <queue>

#include "Address.h"
#include "PendingConnection.h"
#include "Socket.h"
#include "Buffer.h";

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
	Peer(PeerType type, int maxConnections, unsigned int receiveBufferSize, unsigned int sendBufferSize);
	virtual bool StartConcrete() = 0;
	virtual void ProcessMessage(const Message& message, const Address& address) = 0;
	virtual void TickConcrete(float elapsedTime) = 0;
	virtual void DisconnectRemotePeerConcrete(RemotePeer& remotePeer) = 0;
	virtual bool StopConcrete() = 0;

	void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;
	bool AddRemoteClient(const Address& addressInfo, uint16_t id, uint64_t dataPrefix);
	int FindFreeRemoteClientSlot() const;
	RemotePeer* GetRemotePeerFromAddress(const Address& address);
	bool IsRemotePeerAlreadyConnected(const Address& address) const;
	PendingConnection* GetPendingConnectionFromAddress(const Address& address);
	bool IsPendingConnectionAlreadyAdded(const Address& address) const;
	void RemovePendingConnection(const Address& address);
	bool BindSocket(const Address& address) const;

	std::queue<unsigned int> _remotePeerSlotIDsToDisconnect;
	std::vector<PendingConnection> _pendingConnections;
	const int _maxConnections;
	std::vector<bool> _remotePeerSlots;
	std::vector<RemotePeer> _remotePeers;

private:
	bool InitializeSocketsLibrary();

	void ProcessReceivedData();
	void ProcessDatagram(Buffer& buffer, const Address& address);

	void TickRemotePeers(float elapsedTime);
	void HandlerRemotePeersInactivity();

	int GetPendingConnectionIndexFromAddress(const Address& address) const;
	int GetRemotePeerIndexFromAddress(const Address& address) const;

	void SendData();
	void SendPacketToRemotePeer(RemotePeer& remotePeer);
	void SendDataToAddress(const Buffer& buffer, const Address& address) const;

	void StartDisconnectingRemotePeer(unsigned int index);

	void FinishRemotePeersDisconnection();

	PeerType _type;
	Address _address;
	Socket _socket;

	unsigned int _receiveBufferSize;
	uint8_t* _receiveBuffer;
	unsigned int _sendBufferSize;
	uint8_t* _sendBuffer;
};
