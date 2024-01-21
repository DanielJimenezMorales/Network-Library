#include <sstream>

#include "Server.h"
#include "Logger.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "Buffer.h"
#include "PendingConnection.h"
#include "MessageFactory.h"

Server::Server(int maxConnections) : _maxConnections(maxConnections), Peer(PeerType::ServerMode)
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

bool Server::StartConcrete()
{
	LOG_INFO("Server started succesfully!");
	return true;
}

void Server::TickConcrete(float elapsedTime)
{
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

		NetworkPacket packet = NetworkPacket(0);
		Message* message;
		do
		{
			message = _pendingConnections[i].GetAMessage();
			packet.AddMessage(message);
		} while (_pendingConnections[i].ArePendingMessages());

		SendPacketToAddress(packet, _pendingConnections[i].GetAddress());

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

		NetworkPacket packet = NetworkPacket(0);
		Message* message;
		do
		{
			message = _remoteClients[i].GetAMessage();
			packet.AddMessage(message);
		} while (_remoteClients[i].ArePendingMessages());

		Buffer* buffer = new Buffer(packet.Size());
		packet.Write(*buffer);
		SendPacketToRemoteClient(_remoteClients[i], packet);
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
	//TODO Do it in the new way. Create message, store message, send message
	MessageFactory* messageFactory = MessageFactory::GetInstance();

	Message* connectionDeniedMessage = messageFactory->LendMessage(MessageType::ConnectionDenied);

	NetworkPacket packet = NetworkPacket(0);
	packet.AddMessage(connectionDeniedMessage);

	LOG_INFO("Sending connection denied...");
	SendPacketToAddress(packet, address);

	messageFactory->ReleaseMessage(connectionDeniedMessage);
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

void Server::SendPacketToRemoteClient(const RemoteClient& remoteClient, const NetworkPacket& packet) const
{
	SendPacketToAddress(packet, remoteClient.GetAddress());
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

bool Server::StopConcrete()
{
	//TODO: Send disconnect packet to all the connected clients
	return true;
}