#include <sstream>

#include "Socket.h"
#include "Address.h"
#include "Logger.h"

Socket::Socket() : _defaultMTUSize(1500), _listenSocket(INVALID_SOCKET)
{
	
}

int Socket::GetLastError() const
{
	return WSAGetLastError();
}

bool Socket::IsValid() const
{
	return !(_listenSocket == INVALID_SOCKET);
}

bool Socket::SetBlockingMode(bool status)
{
	if (!IsValid())
	{
		return false;
	}

	unsigned long listenSocketBlockingMode = status ? 0 : 1;
	int iResult = ioctlsocket(_listenSocket, FIONBIO, &listenSocketBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Socket error. Error while setting blocking mode, to " << listenSocketBlockingMode  << ". error code " << GetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	return true;
}

bool Socket::Create()
{
	_listenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (!IsValid())
	{
		std::stringstream ss;
		ss << "Socket error. Error while creating the socket, error code " << GetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	return true;
}

bool Socket::Bind(const Address& address)
{
	if (!IsValid())
	{
		return false;
	}

	if (bind(_listenSocket, (sockaddr*)&address.GetInfo(), sizeof(address.GetInfo())) == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Socket error. Error while binding the listen socket, error code " << GetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	return false;
}

bool Socket::Close()
{
	if (!IsValid())
	{
		return false;
	}

	int iResult = closesocket(_listenSocket);
	if (iResult == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Socket error. Error while closing the socket, error code " << GetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	return true;
}

bool Socket::Start()
{
	Create();
	SetBlockingMode(false);
	return true;
}

bool Socket::ArePendingDataToReceive() const
{
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(_listenSocket, &readSet);

	timeval timeval;
	timeval.tv_sec = 0;
	timeval.tv_usec = 0;

	int iResult = select(0, &readSet, NULL, NULL, &timeval);
	if (iResult > 0)
	{
		return true;
	}
	else if (iResult == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Socket error. Error while checking for incoming messages, error code: " << GetLastError();
		LOG_ERROR(ss.str());
	}

	return false;
}

bool Socket::ReceiveFrom(uint8_t* incomingDataBuffer, unsigned int incomingDataBufferSize, Address* remoteAddress, unsigned int& numberOfBytesRead) const
{
	if (incomingDataBuffer == nullptr || !IsValid())
	{
		return false;
	}

	sockaddr_in incomingAddress;
	int incomingAddressSize = sizeof(incomingAddress);
	ZeroMemory(&incomingAddress, incomingAddressSize);

	int bytesIn = recvfrom(_listenSocket, (char*)incomingDataBuffer, incomingDataBufferSize, 0, (sockaddr*)&incomingAddress, &incomingAddressSize);
	if (bytesIn == SOCKET_ERROR)
	{
		int error = GetLastError();
		if (error == WSAEMSGSIZE)
		{
			LOG_ERROR("Socket error. The message received does not fit inside the buffer.");
		}
		else
		{
			std::stringstream ss;
			ss << "Socket error. Error while receiving a message, error code: " << error;
			LOG_ERROR(ss.str());
		}
		return false;
	}

	*remoteAddress = Address(incomingAddress);
	numberOfBytesRead = bytesIn;

	return true;
}

bool Socket::SendTo(const uint8_t* dataBuffer, unsigned int dataBufferSize, const Address& remoteAddress) const
{
	if (dataBuffer == nullptr || !IsValid())
	{
		return false;
	}

	if (dataBufferSize > _defaultMTUSize)
	{
		std::stringstream ss;
		ss << "Socket warning. Trying to send a packet bigger than the MTU size theshold. This could result in Packet Fragmentation and as a consequence worse network conditions. Packet size: " << dataBufferSize << ", MTU size threshold: " << _defaultMTUSize;
		LOG_WARNING(ss.str());
	}

	int addressSize = sizeof(remoteAddress.GetInfo());

	int bytesSent = sendto(_listenSocket, (char*)dataBuffer, dataBufferSize, 0, (sockaddr*)&remoteAddress.GetInfo(), addressSize);
	if (bytesSent == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Socket error. Error while sending data, error code " << GetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	return true;
}

Socket::~Socket()
{
	Close();
}
