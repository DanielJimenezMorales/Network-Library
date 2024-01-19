#include <sstream>
#include "Peer.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "Logger.h"
#include "Buffer.h"
#include "Address.h"

bool Peer::Start()
{
	if (!InitializeSocketsLibrary())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}
	
	if (!CreateSocket())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}

	if(!EnableSocketNonBlockingMode())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}

	if (!BindSocket())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}

	if (!StartConcrete())
	{
		LOG_ERROR("Error while starting peer, aborting operation...");
		return false;
	}
	return true;
}

bool Peer::Tick(float elapsedTime)
{
	if (IsThereNewDataToProcess())
	{
		ProcessReceivedData();
	}

	TickConcrete(elapsedTime);
	return true;
}

bool Peer::Stop()
{
	StopConcrete();
	CloseSocket();
	WSACleanup();

	return true;
}

Peer::~Peer()
{
}

bool Peer::CreateSocket()
{
	_listenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_listenSocket == INVALID_SOCKET)
	{
		std::stringstream ss;
		ss << "Error while creating the listen socket, error code " << WSAGetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	return true;
}

bool Peer::EnableSocketNonBlockingMode()
{
	unsigned long listenSocketBlockingMode = 1;
	int iResult = ioctlsocket(_listenSocket, FIONBIO, &listenSocketBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Error at ioctlsocket, error code " << WSAGetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	return true;
}

void Peer::SendPacketToAddress(const NetworkPacket& packet, const Address& address) const
{
	Buffer* buffer = new Buffer(packet.Size());
	packet.Write(*buffer);

	SendDataToAddress(*buffer, address);

	delete buffer;
}

bool Peer::BindSocket()
{
	int iResult = 0;
	if (_type == PeerType::ClientMode)
	{
		struct addrinfo* result = NULL;
		struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		iResult = getaddrinfo(NULL, "1234", &hints, &result);
		if (iResult != 0)
		{
			LOG_ERROR("Error while getting the address information. Error code: " + iResult);
			return false;
		}

		bind(_listenSocket, result->ai_addr, (int)result->ai_addrlen);
		freeaddrinfo(result);
	}
	else if (_type == PeerType::ServerMode)
	{
		sockaddr_in serverHint;
		serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
		serverHint.sin_family = AF_INET;
		serverHint.sin_port = htons(54000); // Convert from little to big endian

		if (bind(_listenSocket, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
		{
			std::stringstream ss;
			ss << "Error while binding the listen socket, error code " << WSAGetLastError();
			LOG_ERROR(ss.str());
			return false;
		}
	}

	return true;
}

bool Peer::IsThereNewDataToProcess() const
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
		iResult = WSAGetLastError();
		LOG_ERROR("Error while checking for incoming messages, error code " + iResult);
	}

	return false;
}

bool Peer::InitializeSocketsLibrary()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);//Init WS. You need to pass it the version (1.0, 1.1, 2.2...) and a pointer to WSADATA which contains info about the WS impl.
	if (iResult != 0)
	{
		LOG_ERROR("WSAStartup failed: " + iResult);
		return false;
	}

	return true;
}

void Peer::ProcessReceivedData()
{
	Buffer* buffer = nullptr;
	Address address = Address::GetInvalid();
	GetDatagramFromAddress(&buffer, &address);
	ProcessDatagram(*buffer, address);

	delete buffer;
	buffer = nullptr;
}

bool Peer::GetDatagramFromAddress(Buffer** buffer, Address* address)
{
	sockaddr_in incomingAddress;
	int incomingAddressSize = sizeof(incomingAddress);
	ZeroMemory(&incomingAddress, incomingAddressSize);

	int size = 1024;
	uint8_t* data = new uint8_t[size];

	int bytesIn = recvfrom(_listenSocket, (char*)data, size, 0, (sockaddr*)&incomingAddress, &incomingAddressSize);
	if (bytesIn == SOCKET_ERROR)
	{
		LOG_ERROR("Error while receiving a message, error code " + WSAGetLastError());
		return false;
	}

	*buffer = new Buffer(data, bytesIn);
	*address = Address(incomingAddress);
	return true;
}

void Peer::ProcessDatagram(Buffer& buffer, const Address& address)
{
	//Read incoming packet
	NetworkPacket packet = NetworkPacket();
	packet.Read(buffer);

	//Process packet messages one by one
	std::vector<Message*>::const_iterator constIterator = packet.GetMessages();
	unsigned int numberOfMessagesInPacket = packet.GetNumberOfMessages();
	const Message* message = nullptr;
	for (unsigned int i = 0; i < numberOfMessagesInPacket; ++i)
	{
		message = *(constIterator + i);
		ProcessMessage(*message, address);
	}

	//Free memory for those messages in the packet.Read() operation
	packet.ReleaseMessages();
}

bool Peer::CloseSocket()
{
	int iResult = closesocket(_listenSocket);
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		LOG_ERROR("Error while closing the listen socket, error code " + iResult);
		return false;
	}

	return true;
}

void Peer::SendDataToAddress(const Buffer& buffer, const Address& address) const
{
	int bytesSent = sendto(_listenSocket, (char*)buffer.GetData(), buffer.GetSize(), 0, (sockaddr*)&address.GetInfo(), sizeof(address.GetInfo()));
	if (bytesSent == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Error while sending packet to server, error code " << WSAGetLastError();
		LOG_ERROR(ss.str());
	}
}
