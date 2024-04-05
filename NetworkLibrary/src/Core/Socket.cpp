#include <sstream>

#include "Socket.h"
#include "Address.h"
#include "Logger.h"

namespace NetLib
{
	Socket::Socket() : _defaultMTUSize(1500), _listenSocket(INVALID_SOCKET)
	{

	}

	SocketResult Socket::InitializeSocketsLibrary()
	{
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);//Init WS. You need to pass it the version (1.0, 1.1, 2.2...) and a pointer to WSADATA which contains info about the WS impl.
		if (iResult != 0)
		{
			Common::LOG_ERROR("WSAStartup failed: " + iResult);
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	int Socket::GetLastError() const
	{
		return WSAGetLastError();
	}

	bool Socket::IsValid() const
	{
		return !(_listenSocket == INVALID_SOCKET);
	}

	SocketResult Socket::SetBlockingMode(bool status)
	{
		if (!IsValid())
		{
			return SocketResult::SOKT_ERR;
		}

		unsigned long listenSocketBlockingMode = status ? 0 : 1;
		int iResult = ioctlsocket(_listenSocket, FIONBIO, &listenSocketBlockingMode);
		if (iResult == SOCKET_ERROR)
		{
			std::stringstream ss;
			ss << "Socket error. Error while setting blocking mode, to " << listenSocketBlockingMode << ". error code " << GetLastError();
			Common::LOG_ERROR(ss.str());
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::Create()
	{
		_listenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (!IsValid())
		{
			std::stringstream ss;
			ss << "Socket error. Error while creating the socket, error code " << GetLastError();
			Common::LOG_ERROR(ss.str());
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::Bind(const Address& address) const
	{
		if (!IsValid())
		{
			return SocketResult::SOKT_ERR;
		}

		if (bind(_listenSocket, (sockaddr*)&address.GetInfo(), sizeof(address.GetInfo())) == SOCKET_ERROR)
		{
			std::stringstream ss;
			ss << "Socket error. Error while binding the listen socket, error code " << GetLastError();
			Common::LOG_ERROR(ss.str());
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::Close()
	{
		if (!IsValid())
		{
			return SocketResult::SOKT_ERR;
		}

		int iResult = closesocket(_listenSocket);
		if (iResult == SOCKET_ERROR)
		{
			std::stringstream ss;
			ss << "Socket error. Error while closing the socket, error code " << GetLastError();
			Common::LOG_ERROR(ss.str());
			return SocketResult::SOKT_ERR;
		}

		iResult = WSACleanup();
		if (iResult == SOCKET_ERROR)
		{
			std::stringstream ss;
			ss << "Socket error. Error while closing the sockets library, error code " << GetLastError();
			Common::LOG_ERROR(ss.str());
			return SocketResult::SOKT_ERR;
		}

		_listenSocket = INVALID_SOCKET;

		Common::LOG_INFO("Socket succesfully closed");
		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::Start()
	{
		SocketResult result = SocketResult::SOKT_SUCCESS;

		result = InitializeSocketsLibrary();
		if (result != SocketResult::SOKT_SUCCESS)
		{
			Common::LOG_ERROR("Error while starting the sockets library, aborting operation...");
			return result;
		}

		result = Create();
		if (result != SocketResult::SOKT_SUCCESS)
		{
			return result;
		}

		result = SetBlockingMode(false);
		if (result != SocketResult::SOKT_SUCCESS)
		{
			return result;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::ReceiveFrom(uint8_t* incomingDataBuffer, unsigned int incomingDataBufferSize, Address* remoteAddress, unsigned int& numberOfBytesRead) const
	{
		if (incomingDataBuffer == nullptr || !IsValid())
		{
			return SocketResult::SOKT_ERR;
		}

		sockaddr_in incomingAddress;
		int incomingAddressSize = sizeof(incomingAddress);
		ZeroMemory(&incomingAddress, incomingAddressSize);

		int bytesIn = recvfrom(_listenSocket, (char*)incomingDataBuffer, incomingDataBufferSize, 0, (sockaddr*)&incomingAddress, &incomingAddressSize);
		*remoteAddress = Address(incomingAddress);

		if (bytesIn == SOCKET_ERROR)
		{
			int error = GetLastError();

			if (error == WSAEMSGSIZE)
			{
				Common::LOG_ERROR("Socket error. The message received does not fit inside the buffer.");
				return SocketResult::SOKT_ERR;
			}
			else if (error == WSAEWOULDBLOCK)
			{
				return SocketResult::SOKT_WOULDBLOCK;
			}
			else if (error == WSAECONNRESET)
			{
				Common::LOG_WARNING("Socket warning. The remote socket has been closed unexpectly.");
				return SocketResult::SOKT_CONNRESET;
			}
			else
			{
				std::stringstream ss;
				ss << "Socket error. Error while receiving a message, error code: " << error;
				Common::LOG_ERROR(ss.str());
				return SocketResult::SOKT_ERR;
			}
		}

		numberOfBytesRead = bytesIn;

		return SocketResult::SOKT_SUCCESS;
	}

	SocketResult Socket::SendTo(const uint8_t* dataBuffer, unsigned int dataBufferSize, const Address& remoteAddress) const
	{
		if (dataBuffer == nullptr || !IsValid())
		{
			return SocketResult::SOKT_ERR;
		}

		if (dataBufferSize > _defaultMTUSize)
		{
			std::stringstream ss;
			ss << "Socket warning. Trying to send a packet bigger than the MTU size theshold. This could result in Packet Fragmentation and as a consequence worse network conditions. Packet size: " << dataBufferSize << ", MTU size threshold: " << _defaultMTUSize;
			Common::LOG_WARNING(ss.str());
		}

		int addressSize = sizeof(remoteAddress.GetInfo());

		int bytesSent = sendto(_listenSocket, (char*)dataBuffer, dataBufferSize, 0, (sockaddr*)&remoteAddress.GetInfo(), addressSize);
		if (bytesSent == SOCKET_ERROR)
		{
			std::stringstream ss;
			ss << "Socket error. Error while sending data, error code " << GetLastError();
			Common::LOG_ERROR(ss.str());
			return SocketResult::SOKT_ERR;
		}

		return SocketResult::SOKT_SUCCESS;
	}

	Socket::~Socket()
	{
		Close();
	}
}
