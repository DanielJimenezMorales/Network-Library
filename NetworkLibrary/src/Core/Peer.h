#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>

class Address;
class Buffer;
class Message;

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

protected:
	Peer(PeerType type) : _type(type), _listenSocket(INVALID_SOCKET) {}
private:
	bool CreateSocket();
	bool EnableSocketNonBlockingMode();
	bool BindSocket();
	virtual bool StartConcrete() = 0;

	bool IsThereNewDataToProcess() const;
	void ProcessReceivedData();
	bool GetDatagramFromAddress(Buffer** buffer, Address* address);
	void ProcessDatagram(Buffer& buffer, const Address& address);
	virtual void ProcessMessage(const Message& message, const Address& address) = 0;

	virtual void TickConcrete(float elapsedTime) = 0;

	virtual bool StopConcrete() = 0;
	bool CloseSocket();

	SOCKET _listenSocket;
	PeerType _type;
};
