#pragma once
#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>

class Address;

class Socket
{
public:
	Socket();

	bool Start();
	bool Bind(const Address& address);
	bool ArePendingDataToReceive() const;
	bool ReceiveFrom(uint8_t* incomingDataBuffer, unsigned int incomingDataBufferSize, Address* remoteAddress, unsigned int& numberOfBytesRead) const;
	bool SendTo(const uint8_t* dataBuffer, unsigned int dataBufferSize, const Address& remoteAddress) const;
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

