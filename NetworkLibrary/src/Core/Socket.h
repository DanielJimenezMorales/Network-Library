#pragma once
#include "NumericTypes.h"
#include <winsock2.h>
#include <ws2tcpip.h>

namespace NetLib
{
	class Address;

	enum SocketResult : uint8
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
		SocketResult ReceiveFrom(uint8* incomingDataBuffer, uint32 incomingDataBufferSize, Address* remoteAddress, uint32& numberOfBytesRead) const;
		SocketResult SendTo(const uint8* dataBuffer, uint32 dataBufferSize, const Address& remoteAddress) const;
		SocketResult Close();

		~Socket();

	private:
		SocketResult InitializeSocketsLibrary();
		int32 GetLastError() const;
		bool IsValid() const;
		SocketResult SetBlockingMode(bool status);
		SocketResult Create();

		uint32 _defaultMTUSize;
		SOCKET _listenSocket;
	};
}
