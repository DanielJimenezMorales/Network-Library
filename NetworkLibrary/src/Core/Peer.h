#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>

#include "Address.h"
#include "Socket.h"

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
	Peer(PeerType type) : _type(type), _socket(), _address(Address::GetInvalid()) {}
	virtual bool StartConcrete() = 0;
	virtual void ProcessMessage(const Message& message, const Address& address) = 0;
	virtual void TickConcrete(float elapsedTime) = 0;
	virtual bool StopConcrete() = 0;

	void SendPacketToAddress(const NetworkPacket& packet, const Address& address) const;


private:
	bool InitializeSocketsLibrary();
	bool BindSocket();

	bool IsThereNewDataToProcess() const;
	void ProcessReceivedData();
	bool GetDatagramFromAddress(Buffer** buffer, Address* address);
	void ProcessDatagram(Buffer& buffer, const Address& address);

	void SendDataToAddress(const Buffer& buffer, const Address& address) const;

	PeerType _type;
	Address _address;
	Socket _socket;
};
