#include <sstream>
#include <cassert>

#include "Server.h"
#include "Logger.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "PendingConnection.h"
#include "MessageFactory.h"

#define SERVER_PORT 54000

Server::Server(int maxConnections) : Peer(PeerType::ServerMode, maxConnections, 1024, 1024)
{
	
}

Server::~Server()
{
}

bool Server::StartConcrete()
{
	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(SERVER_PORT); // Convert from little to big endian
	Address address = Address(serverHint);
	BindSocket(address);

	LOG_INFO("Server started succesfully!");
	return true;
}

void Server::TickConcrete(float elapsedTime)
{
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
	case MessageType::InGame:
	{
		const InGameMessage& inGameMessage = static_cast<const InGameMessage&>(message);
		ProcessInGame(inGameMessage, address);
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
		//int connectedClientIndex = FindExistingClientIndex(address);
		RemotePeer* remoteClient = GetRemotePeerFromAddress(address);
		CreateConnectionApprovedMessage(*remoteClient);
		LOG_INFO("The client is already connected, sending connection approved...");
	}
	else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
	{
		SendConnectionDeniedPacket(address);
		LOG_WARNING("All available connection slots are full. Denying incoming connection...");
	}
}

void Server::CreateDisconnectionMessage(RemotePeer& remoteClient)
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

void Server::CreateInGameResponseMessage(RemotePeer& remoteClient, uint64_t data)
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	Message* message = messageFactory->LendMessage(MessageType::InGameResponse);
	if (message == nullptr)
	{
		LOG_ERROR("Can't create new in game response Message because the MessageFactory has returned a null message");
		return;
	}

	message->SetReliability(true);

	InGameResponseMessage* inGameResponseMessage = static_cast<InGameResponseMessage*>(message);

	inGameResponseMessage->data = data;
	remoteClient.AddMessage(inGameResponseMessage);

	LOG_INFO("In game response message created.");
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
	MessageFactory* messageFactory = MessageFactory::GetInstance();

	Message* connectionDeniedMessage = messageFactory->LendMessage(MessageType::ConnectionDenied);

	NetworkPacket packet = NetworkPacket();
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
			int availableClientSlot = FindFreeRemoteClientSlot();
			AddNewRemoteClient(availableClientSlot, address, dataPrefix);

			//Delete pending connection since we have accepted
			_pendingConnections.erase(_pendingConnections.begin() + pendingConnectionIndex);

			//Send connection approved packet
			CreateConnectionApprovedMessage(_remotePeers[availableClientSlot]);
			LOG_INFO("Connection approved");
		}
	}
	else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
	{
		//Find remote client
		RemotePeer* remoteClient = GetRemotePeerFromAddress(address);

		//Check if data prefix match
		if (remoteClient->GetDataPrefix() != dataPrefix)
		{
			return;
		}

		CreateConnectionApprovedMessage(*remoteClient);
	}
	else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
	{
		SendConnectionDeniedPacket(address);
	}
}

void Server::ProcessInGame(const InGameMessage& message, const Address& address)
{
	std::stringstream ss;
	ss << "InGame ID: " << message.data;
	LOG_INFO(ss.str());

	RemotePeer* remotePeer = GetRemotePeerFromAddress(address);
	assert(remotePeer != nullptr);

	CreateInGameResponseMessage(*remotePeer, message.data);
}

int Server::IsClientAbleToConnect(const Address& address) const
{
	if (IsRemotePeerAlreadyConnected(address))
	{
		return 1;
	}

	int availableClientSlot = FindFreeRemoteClientSlot();
	if (availableClientSlot == -1)
	{
		return -1;
	}

	return 0;
}

void Server::AddNewRemoteClient(int remoteClientSlotIndex, const Address& address, uint64_t dataPrefix)
{
	_remotePeerSlots[remoteClientSlotIndex] = true;
	_remotePeers[remoteClientSlotIndex].Connect(address.GetInfo(), _nextAssignedRemoteClientID, REMOTE_CLIENT_INACTIVITY_TIME, dataPrefix);
	++_nextAssignedRemoteClientID;
}

void Server::CreateConnectionApprovedMessage(RemotePeer& remoteClient)
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

void Server::SendPacketToRemoteClient(const RemotePeer& remoteClient, const NetworkPacket& packet) const
{
	SendPacketToAddress(packet, remoteClient.GetAddress());
}

void Server::DisconnectRemotePeerConcrete(RemotePeer& remotePeer)
{
	CreateDisconnectionMessage(remotePeer);
}

bool Server::StopConcrete()
{
	//TODO: Send disconnect packet to all the connected clients
	return true;
}