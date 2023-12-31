#include "Server.h"

Server::Server(int maxConnections) : _maxConnections(maxConnections)
{
	_remoteClientSlots.reserve(_maxConnections);
	_remoteClients.reserve(_maxConnections);
	_pendingConnections.reserve(_maxConnections * 2); //There could be more pending connections than clients

	for (size_t i = 0; i < _maxConnections; ++i)
	{
		_remoteClientSlots.push_back(false);
		_remoteClients.push_back(nullptr);
	}
}

Server::~Server()
{
	for (size_t i = 0; i < _maxConnections; ++i)
	{
		if (_remoteClientSlots[i])
		{
			delete _remoteClients[i];
			_remoteClients[i] = nullptr;
		}
	}
}

bool Server::Start()
{
	LOG_INFO("Starting server...");

	if (!InitSocket())
	{
		return false;
	}

	LOG_INFO("Server started succesfully!");
	return true;
}

bool Server::InitSocket()
{
	int iResult = 0;

	//Create socket
	_listenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_listenSocket == INVALID_SOCKET)
	{
		std::stringstream ss;
		ss << "Error while creating the listen socket, error code " << WSAGetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	//Set socket to non blocking
	unsigned long listenSocketBlockingMode = 1;
	iResult = ioctlsocket(_listenSocket, FIONBIO, &listenSocketBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Error at ioctlsocket, error code " << WSAGetLastError();
		LOG_ERROR(ss.str());
		return false;
	}

	//Bind socket
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

	return true;
}

void Server::Tick(float elapsedTime)
{
	if (IsThereNewDataToProcess())
	{
		ProcessReceivedData();
	}

	HandleConnectedClientsInactivity(elapsedTime);
}

void Server::HandleConnectedClientsInactivity(float elapsedTime)
{
	for (unsigned int i = 0; i < _maxConnections; ++i)
	{
		if (_remoteClientSlots[i])
		{
			_remoteClients[i]->Tick(elapsedTime);
			if (_remoteClients[i]->IsInactive())
			{
				DisconnectRemoteClient(i);
			}
		}
	}
}

uint64_t Server::GenerateServerSalt() const
{
	//TODO Change this in order to get another random generator that generates 64bit numbers
	srand(time(NULL) + 3589);
	uint64_t serverSalt = rand();
	return serverSalt;
}

bool Server::IsThereNewDataToProcess() const
{
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(_listenSocket, &readSet);

	//Create a timeval to set sec and ms to 0 so this will be a non blocking operation
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
		ss << "Error while checking for incoming packets, error code " << WSAGetLastError();
		LOG_ERROR(ss.str());
	}

	return false;
}

void Server::ProcessReceivedData()
{
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);

	Buffer* buffer = new Buffer(1024);
	int bytesIn = recvfrom(_listenSocket, (char*)buffer->data, 1024, 0, (sockaddr*)&client, &clientLength);
	if (bytesIn == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Error while receiving data, error code " << WSAGetLastError();
		LOG_ERROR(ss.str());
		return;
	}

	Address address = Address(client);
	ProcessDatagram(*buffer, address);

	delete buffer;
	buffer = nullptr;
}

void Server::ProcessDatagram(Buffer& buffer, const Address& address)
{
	uint8_t packetType = BufferUtils::ReadByte(buffer);

	switch (packetType)
	{
	case NetworkPacketType::ConnectionRequest:
		ProcessConnectionRequest(buffer, address);
		break;
	case NetworkPacketType::ConnectionChallengeResponse:
		ProcessConnectionChallengeResponse(buffer, address);
		break;
	}
}

void Server::ProcessConnectionRequest(Buffer& buffer, const Address& address)
{
	std::stringstream ss;
	ss << "Processing connection request from [IP: " << address.GetIP() << ", Port: " << address.GetPort() << "]";
	LOG_INFO(ss.str());

	int isAbleToConnectResult = IsClientAbleToConnect(address);

	if (isAbleToConnectResult == 0)//If there is green light keep with the connection pipeline.
	{
		uint64_t clientSalt = BufferUtils::ReadLong(buffer);
		int pendingConnectionIndex = -1;
		for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
		{
			if (_pendingConnections[i].address == address && _pendingConnections[i].clientSalt == clientSalt)
			{
				pendingConnectionIndex = i;
				break;
			}
		}

		if (pendingConnectionIndex == -1) //If no pending connection was found create one!
		{
			PendingConnectionData newPendingConnection(address);
			newPendingConnection.clientSalt = clientSalt;
			newPendingConnection.serverSalt = GenerateServerSalt();
			_pendingConnections.push_back(newPendingConnection);

			pendingConnectionIndex = _pendingConnections.size() - 1;

			std::stringstream ss;
			ss << "Creating a pending connection entry. Client salt: " << clientSalt << " Server salt: " << newPendingConnection.serverSalt;
			LOG_INFO(ss.str());
		}

		SendConnectionChallengePacket(address, pendingConnectionIndex);
	}
	else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
	{
		int connectedClientIndex = FindExistingClientIndex(address);
		SendConnectionApprovedPacketToRemoteClient(*_remoteClients[connectedClientIndex]);
		LOG_INFO("The client is already connected, sending connection approved...");
	}
	else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
	{
		SendConnectionDeniedPacket(address);
		LOG_WARNING("All available connection slots are full. Denying incoming connection...");
	}
}

void Server::SendDisconnectionPacketToRemoteClient(const RemoteClient& remoteClient) const
{
	NetworkDisconnectionPacket disconnectionPacket;
	disconnectionPacket.prefix = remoteClient.GetDataPrefix();
	Buffer* disconnectionBuffer = new Buffer(sizeof(disconnectionPacket));
	disconnectionPacket.Write(*disconnectionBuffer);

	LOG_INFO("Sending disconnection packet...");
	SendDatagramToRemoteClient(remoteClient, *disconnectionBuffer);

	delete disconnectionBuffer;
	disconnectionBuffer = nullptr;
}

void Server::SendConnectionChallengePacket(const Address& address, int pendingConnectionIndex) const
{
	NetworkConnectionChallengePacket connectionChallengePacket;
	connectionChallengePacket.clientSalt = _pendingConnections[pendingConnectionIndex].clientSalt;
	connectionChallengePacket.serverSalt = _pendingConnections[pendingConnectionIndex].serverSalt;

	Buffer* connectionChallengeBuffer = new Buffer(sizeof(connectionChallengePacket));
	connectionChallengePacket.Write(*connectionChallengeBuffer);

	LOG_INFO("Sending connection challenge...");
	SendDataToAddress(*connectionChallengeBuffer, address);

	delete connectionChallengeBuffer;
	connectionChallengeBuffer = nullptr;
}

void Server::SendConnectionDeniedPacket(const Address& address) const
{
	NetworkConnectionDeniedPacket connectionDeniedPacket;
	Buffer* connectionDeniedBuffer = new Buffer(sizeof(connectionDeniedPacket));
	connectionDeniedPacket.Write(*connectionDeniedBuffer);

	LOG_INFO("Sending connection denied...");
	SendDataToAddress(*connectionDeniedBuffer, address);

	delete connectionDeniedBuffer;
	connectionDeniedBuffer = nullptr;
}

void Server::ProcessConnectionChallengeResponse(Buffer& buffer, const Address& address)
{
	std::stringstream ss;
	ss << "Processing connection challenge response from [IP: " << address.GetIP() << ", Port: " << address.GetPort() << "]";
	LOG_INFO(ss.str());

	uint64_t dataPrefix = BufferUtils::ReadLong(buffer);

	int isAbleToConnectResult = IsClientAbleToConnect(address);

	if (isAbleToConnectResult == 0)//If there is green light keep with the connection pipeline.
	{
		//Search for a pending connection that matches the challenge response
		int pendingConnectionIndex = -1;
		for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
		{
			if (_pendingConnections[i].GetPrefix() == dataPrefix && address == _pendingConnections[i].address)
			{
				pendingConnectionIndex = i;
				break;
			}
		}

		if (pendingConnectionIndex == -1)
		{
			LOG_INFO("Connection denied due to not pending connection found.");
			SendConnectionDeniedPacket(address);
		}
		else
		{
			//Create remote client
			int availableClientSlot = GetEmptyClientSlot();
			AddNewRemoteClient(availableClientSlot, address, dataPrefix);

			//Delete pending connection since we have accepted
			_pendingConnections.erase(_pendingConnections.begin() + pendingConnectionIndex);

			//Senc connection approved packet
			SendConnectionApprovedPacketToRemoteClient(*_remoteClients[availableClientSlot]);
			LOG_INFO("Connection approved");
		}
	}
	else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
	{
		//Find remote client
		int targetClientIndex = FindExistingClientIndex(address);

		//Check if data prefix match
		if (_remoteClients[targetClientIndex]->GetDataPrefix() != dataPrefix)
		{
			return;
		}

		SendConnectionApprovedPacketToRemoteClient(*_remoteClients[targetClientIndex]);
	}
	else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
	{
		SendConnectionDeniedPacket(address);
	}
}

int Server::IsClientAbleToConnect(const Address& address) const
{
	if (IsClientAlreadyConnected(address))
	{
		return 1;
	}

	int availableClientSlot = GetEmptyClientSlot();
	if (availableClientSlot == -1)
	{
		return -1;
	}

	return 0;
}

int Server::GetEmptyClientSlot() const
{
	for (int i = 0; i < _maxConnections; ++i)
	{
		if (!_remoteClientSlots[i])
		{
			return i;
		}
	}

	return -1;
}

bool Server::IsClientAlreadyConnected(const Address& address) const
{
	for (size_t i = 0; i < _maxConnections; ++i)
	{
		if (_remoteClientSlots[i])
		{
			if (_remoteClients[i]->IsAddressEqual(address))
			{
				return true;
			}
		}
	}

	return false;
}

void Server::AddNewRemoteClient(int remoteClientSlotIndex, const Address& address, uint64_t dataPrefix)
{
	_remoteClientSlots[remoteClientSlotIndex] = true;
	_remoteClients[remoteClientSlotIndex] = new RemoteClient(address.GetInfo(), _nextAssignedRemoteClientIndex, REMOTE_CLIENT_INACTIVITY_TIME, dataPrefix);
	++_nextAssignedRemoteClientIndex;
}

int Server::FindExistingClientIndex(const Address& address) const
{
	for (size_t i = 0; i < _maxConnections; ++i)
	{
		if (_remoteClientSlots[i])
		{
			if (_remoteClients[i]->GetAddress() == address)
			{
				return i;
			}
		}
	}
	return -1;
}

void Server::SendConnectionApprovedPacketToRemoteClient(const RemoteClient& remoteClient) const
{
	//Write connection packet data into buffer
	NetworkConnectionAcceptedPacket connectionAcceptedPacket;
	connectionAcceptedPacket.prefix = remoteClient.GetDataPrefix();
	connectionAcceptedPacket.clientIndexAssigned = remoteClient.GetClientIndex();
	int connectionAcceptedPacketSize = sizeof(connectionAcceptedPacket);

	Buffer* buffer = new Buffer(connectionAcceptedPacketSize);
	connectionAcceptedPacket.Write(*buffer);

	//Send connection approved packet to remote client
	SendDatagramToRemoteClient(remoteClient, *buffer);

	//Free memory
	delete buffer;
	buffer = nullptr;
}

void Server::SendDatagramToRemoteClient(const RemoteClient& remoteClient, const Buffer& buffer) const
{
	SendDataToAddress(buffer, remoteClient.GetAddress());
}

void Server::SendDataToAddress(const Buffer& buffer, const Address& address) const
{
	int iResult = 0;
	iResult = sendto(_listenSocket, (char*)buffer.data, buffer.size, 0, (sockaddr*)&address.GetInfo(), sizeof(address.GetInfo()));
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		LOG_ERROR("Error while sending data, error code " + iResult);
	}
}

void Server::DisconnectRemoteClient(unsigned int index)
{
	if (!_remoteClients[index])
	{
		return;
	}

	//Send disconnection packet
	SendDisconnectionPacketToRemoteClient(*_remoteClients[index]);

	_remoteClientSlots[index] = false;

	delete _remoteClients[index];
	_remoteClients[index] = nullptr;
}

int Server::Stop()
{
	LOG_INFO("Stopping client...");

	int iResult = 0;
	iResult = closesocket(_listenSocket);
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		LOG_ERROR("Error while closing the listen socket, error code " + iResult);
	}

	//TODO: Send disconnect packet to all the connected clients

	return iResult;
}