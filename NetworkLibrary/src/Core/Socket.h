#pragma once
#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace NetLib
{
	class Address;

	enum SocketResult : uint8_t
	{
		SOKT_ERR = 0,
		SOKT_SUCCESS = 1,
		SOKT_WOULDBLOCK = 2,
		SOKT_CONNRESET = 3
	};

	class Socket
	{
	public:
		Socket();

		SocketResult Start();
		SocketResult Bind(const Address& address) const;
		SocketResult ReceiveFrom(uint8_t* incomingDataBuffer, unsigned int incomingDataBufferSize, Address* remoteAddress, unsigned int& numberOfBytesRead) const;
		SocketResult SendTo(const uint8_t* dataBuffer, unsigned int dataBufferSize, const Address& remoteAddress) const;
		SocketResult Close();

		~Socket();

	private:
		SocketResult InitializeSocketsLibrary();
		int GetLastError() const;
		bool IsValid() const;
		SocketResult SetBlockingMode(bool status);
		SocketResult Create();

		unsigned int _defaultMTUSize;
		SOCKET _listenSocket;
	};
}
