#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>

class Address;
class Buffer;
class Message;
class NetworkPacket;

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
	Peer(PeerType type) : _type(type), _listenSocket(INVALID_SOCKET) {}
	virtual bool StartConcrete() = 0;
	virtual void ProcessMessage(const Message& message, const Address& address) = 0;
	virtual void TickConcrete(float elapsedTime) = 0;
	virtual bool StopConcrete() = 0;

	void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;

	SOCKET _listenSocket;

private:
	bool InitializeSocketsLibrary();
	bool CreateSocket();
	bool EnableSocketNonBlockingMode();
	bool BindSocket();

	bool IsThereNewDataToProcess() const;
	void ProcessReceivedData();
	bool GetDatagramFromAddress(Buffer** buffer, Address* address);
	void ProcessDatagram(Buffer& buffer, const Address& address);

	void SendDataToAddress(const Buffer& buffer, const Address& address) const;

	bool CloseSocket();

	PeerType _type;
};
