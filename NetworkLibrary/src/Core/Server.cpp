#include <sstream>

#include "Server.h"
#include "Logger.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "Buffer.h"
#include "PendingConnection.h"
#include "MessageFactory.h"

Server::Server(int maxConnections) : _maxConnections(maxConnections)
{
	_remoteClientSlots.reserve(_maxConnections);
	_remoteClients.reserve(_maxConnections);
	_pendingConnections.reserve(_maxConnections * 2); //There could be more pending connections than clients

	for (size_t i = 0; i < _maxConnections; ++i)
	{
		_remoteClientSlots.push_back(false);
		_remoteClients.push_back(RemoteClient());
	}
}

Server::~Server()
{
	_remoteClientSlots.clear();
	_remoteClients.clear();
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

	SendData();

	FinishRemoteClientsDisconnection();
}

void Server::HandleConnectedClientsInactivity(float elapsedTime)
{
	for (unsigned int i = 0; i < _maxConnections; ++i)
	{
		if (_remoteClientSlots[i])
		{
			_remoteClients[i].Tick(elapsedTime);
			if (_remoteClients[i].IsInactive())
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

	int size = 1024;
	uint8_t* data = new uint8_t[size];

	int bytesIn = recvfrom(_listenSocket, (char*)data, size, 0, (sockaddr*)&client, &clientLength);
	if (bytesIn == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Error while receiving data, error code " << WSAGetLastError();
		LOG_ERROR(ss.str());
		return;
	}

	Buffer* buffer = new Buffer(data, bytesIn);
	Address address = Address(client);
	ProcessDatagram(*buffer, address);

	delete buffer;
	buffer = nullptr;
}

void Server::ProcessDatagram(Buffer& buffer, const Address& address)
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

	packet.ReleaseMessages();
}

void Server::ProcessMessage(const Message& message, const Address& address)
{
	MessageType messageType = message.GetHeader().type;

	switch (messageType)
	{
	case MessageType::ConnectionRequest:
	{
		const ConnectionRequestMessage& connectionRequestMessage = static_cast<const ConnectionRequestMessage&>(message);
		ProcessConnectionRequest(connectionRequestMessage, address);
		break;
	}
	case MessageType::ConnectionChallengeResponse:
	{
		const ConnectionChallengeResponseMessage& connectionChallengeResponseMessage = static_cast<const ConnectionChallengeResponseMessage&>(message);
		ProcessConnectionChallengeResponse(connectionChallengeResponseMessage, address);
		break;
	}
	default:
		LOG_WARNING("Invalid Message type, ignoring it...");
		break;
	}
}

void Server::ProcessConnectionRequest(const ConnectionRequestMessage& message, const Address& address)
{
	std::stringstream ss;
	ss << "Processing connection request from [IP: " << address.GetIP() << ", Port: " << address.GetPort() << "]";
	LOG_INFO(ss.str());

	int isAbleToConnectResult = IsClientAbleToConnect(address);

	if (isAbleToConnectResult == 0)//If there is green light keep with the connection pipeline.
	{
		uint64_t clientSalt = message.clientSalt;
		int pendingConnectionIndex = -1;
		for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
		{
			if (_pendingConnections[i].IsAddressEqual(address) && _pendingConnections[i].GetClientSalt() == clientSalt)
			{
				pendingConnectionIndex = i;
				break;
			}
		}

		if (pendingConnectionIndex == -1) //If no pending connection was found create one!
		{
			PendingConnection newPendingConnection(address);
			newPendingConnection.SetClientSalt(clientSalt);
			newPendingConnection.SetServerSalt(GenerateServerSalt());
			_pendingConnections.push_back(newPendingConnection);

			pendingConnectionIndex = _pendingConnections.size() - 1;

			std::stringstream ss;
			ss << "Creating a pending connection entry. Client salt: " << clientSalt << " Server salt: " << newPendingConnection.GetServerSalt();
			LOG_INFO(ss.str());
		}

		CreateConnectionChallengeMessage(address, pendingConnectionIndex);
	}
	else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
	{
		int connectedClientIndex = FindExistingClientIndex(address);
		CreateConnectionApprovedMessage(_remoteClients[connectedClientIndex]);
		LOG_INFO("The client is already connected, sending connection approved...");
	}
	else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
	{
		SendConnectionDeniedPacket(address);
		LOG_WARNING("All available connection slots are full. Denying incoming connection...");
	}
}

void Server::SendData()
{
	for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
	{
		if (!_pendingConnections[i].ArePendingMessages())
		{
			continue;
		}

		NetworkPacket packet = NetworkPacket();
		Message* message;
		do
		{
			message = _pendingConnections[i].GetAMessage();
			packet.AddMessage(message);
		} while (_pendingConnections[i].ArePendingMessages());

		Buffer* buffer = new Buffer(packet.Size());
		packet.Write(*buffer);
		SendDataToAddress(*buffer, _pendingConnections[i].GetAddress());
		delete buffer;

		_pendingConnections[i].FreeSentMessages();
	}

	for (unsigned int i = 0; i < _remoteClientSlots.size(); ++i)
	{
		if (!_remoteClientSlots[i])
		{
			continue;
		}

		if (!_remoteClients[i].ArePendingMessages())
		{
			continue;
		}

		NetworkPacket packet = NetworkPacket();
		Message* message;
		do
		{
			message = _remoteClients[i].GetAMessage();
			packet.AddMessage(message);
		} while (_remoteClients[i].ArePendingMessages());

		Buffer* buffer = new Buffer(packet.Size());
		packet.Write(*buffer);
		SendDataToAddress(*buffer, _remoteClients[i].GetAddress());
		delete buffer;

		_remoteClients[i].FreeSentMessages();
	}
}

void Server::CreateDisconnectionMessage(RemoteClient& remoteClient)
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	Message* message = messageFactory->LendMessage(MessageType::Disconnection);
	if (message == nullptr)
	{
		LOG_ERROR("Can't create new Disconnection Message because the MessageFactory has returned a null message");
		return;
	}

	DisconnectionMessage* disconnectionMessage = static_cast<DisconnectionMessage*>(message);

	disconnectionMessage->prefix = remoteClient.GetDataPrefix();
	remoteClient.AddMessage(disconnectionMessage);

	LOG_INFO("Disconnection message created.");
}

void Server::CreateConnectionChallengeMessage(const Address& address, int pendingConnectionIndex)
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	Message* message = messageFactory->LendMessage(MessageType::ConnectionChallenge);
	if (message == nullptr)
	{
		LOG_ERROR("Can't create new Connection Challenge Message because the MessageFactory has returned a null message");
		return;
	}

	ConnectionChallengeMessage* connectionChallengePacket = static_cast<ConnectionChallengeMessage*>(message);
	connectionChallengePacket->clientSalt = _pendingConnections[pendingConnectionIndex].GetClientSalt();
	connectionChallengePacket->serverSalt = _pendingConnections[pendingConnectionIndex].GetServerSalt();
	_pendingConnections[pendingConnectionIndex].AddMessage(connectionChallengePacket);

	LOG_INFO("Connection challenge message created.");
}

void Server::SendConnectionDeniedPacket(const Address& address) const
{
	ConnectionDeniedMessage connectionDeniedPacket;
	Buffer* connectionDeniedBuffer = new Buffer(sizeof(connectionDeniedPacket));
	connectionDeniedPacket.Write(*connectionDeniedBuffer);

	LOG_INFO("Sending connection denied...");
	SendDataToAddress(*connectionDeniedBuffer, address);

	delete connectionDeniedBuffer;
	connectionDeniedBuffer = nullptr;
}

void Server::ProcessConnectionChallengeResponse(const ConnectionChallengeResponseMessage& message, const Address& address)
{
	std::stringstream ss;
	ss << "Processing connection challenge response from [IP: " << address.GetIP() << ", Port: " << address.GetPort() << "]";
	LOG_INFO(ss.str());

	uint64_t dataPrefix = message.prefix;

	int isAbleToConnectResult = IsClientAbleToConnect(address);

	if (isAbleToConnectResult == 0)//If there is green light keep with the connection pipeline.
	{
		//Search for a pending connection that matches the challenge response
		int pendingConnectionIndex = -1;
		for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
		{
			if (_pendingConnections[i].GetPrefix() == dataPrefix && _pendingConnections[i].IsAddressEqual(address))
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

			//Send connection approved packet
			CreateConnectionApprovedMessage(_remoteClients[availableClientSlot]);
			LOG_INFO("Connection approved");
		}
	}
	else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
	{
		//Find remote client
		int targetClientIndex = FindExistingClientIndex(address);

		//Check if data prefix match
		if (_remoteClients[targetClientIndex].GetDataPrefix() != dataPrefix)
		{
			return;
		}

		CreateConnectionApprovedMessage(_remoteClients[targetClientIndex]);
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
			if (_remoteClients[i].IsAddressEqual(address))
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
	_remoteClients[remoteClientSlotIndex].Connect(address.GetInfo(), _nextAssignedRemoteClientID, REMOTE_CLIENT_INACTIVITY_TIME, dataPrefix);
	++_nextAssignedRemoteClientID;
}

int Server::FindExistingClientIndex(const Address& address) const
{
	for (size_t i = 0; i < _maxConnections; ++i)
	{
		if (_remoteClientSlots[i])
		{
			if (_remoteClients[i].GetAddress() == address)
			{
				return i;
			}
		}
	}
	return -1;
}

void Server::CreateConnectionApprovedMessage(RemoteClient& remoteClient)
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	Message* message = messageFactory->LendMessage(MessageType::ConnectionAccepted);
	if (message == nullptr)
	{
		LOG_ERROR("Can't create new Connection Accepted Message because the MessageFactory has returned a null message");
		return;
	}

	ConnectionAcceptedMessage* connectionAcceptedPacket = static_cast<ConnectionAcceptedMessage*>(message);
	connectionAcceptedPacket->prefix = remoteClient.GetDataPrefix();
	connectionAcceptedPacket->clientIndexAssigned = remoteClient.GetClientIndex();
	remoteClient.AddMessage(connectionAcceptedPacket);
}

void Server::SendDatagramToRemoteClient(const RemoteClient& remoteClient, const Buffer& buffer) const
{
	SendDataToAddress(buffer, remoteClient.GetAddress());
}

void Server::SendDataToAddress(const Buffer& buffer, const Address& address) const
{
	int iResult = 0;
	iResult = sendto(_listenSocket, (char*)buffer.GetData(), buffer.GetSize(), 0, (sockaddr*)&address.GetInfo(), sizeof(address.GetInfo()));
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		LOG_ERROR("Error while sending data, error code " + iResult);
	}
}

void Server::DisconnectRemoteClient(unsigned int index)
{
	if (!_remoteClientSlots[index])
	{
		return;
	}

	CreateDisconnectionMessage(_remoteClients[index]);

	_remoteClientSlotIDsToDisconnect.push(index);
}

void Server::FinishRemoteClientsDisconnection()
{
	unsigned int remoteClientSlot;
	while (!_remoteClientSlotIDsToDisconnect.empty())
	{
		remoteClientSlot = _remoteClientSlotIDsToDisconnect.front();
		_remoteClientSlotIDsToDisconnect.pop();

		if (_remoteClientSlots[remoteClientSlot])
		{
			_remoteClientSlots[remoteClientSlot] = false;
			_remoteClients[remoteClientSlot].Disconnect();
		}
	}
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