#pragma once
#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>

class Address;

enum SocketResult : uint8_t
{
	ERR = 0,
	SUCCESS = 1,
	WOULDBLOCK = 2,
	CONNRESET = 3
};

class Socket
{
public:
	Socket();

	bool Start();
	bool Bind(const Address& address) const;
	SocketResult ReceiveFrom(uint8_t* incomingDataBuffer, unsigned int incomingDataBufferSize, Address* remoteAddress, unsigned int& numberOfBytesRead) const;
	SocketResult SendTo(const uint8_t* dataBuffer, unsigned int dataBufferSize, const Address& remoteAddress) const;
	bool Close();

	~Socket();

private:
	int GetLastError() const;
	bool IsValid() const;
	bool SetBlockingMode(bool status);
	bool Create();

	unsigned int _defaultMTUSize;
	SOCKET _listenSocket;
};

