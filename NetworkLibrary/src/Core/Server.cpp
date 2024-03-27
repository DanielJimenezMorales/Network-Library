#include <sstream>
#include <cassert>
#include <memory>

#include "Server.h"
#include "Logger.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "PendingConnection.h"
#include "MessageFactory.h"
#include "TimeClock.h"

#define SERVER_PORT 54000

namespace NetLib
{
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
		Common::LOG_INFO("Server started succesfully!");

		ExecuteOnPeerConnected();
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
		std::stringstream ss;
		ss << "Me proceso type " << static_cast<int>(messageType);
		Common::LOG_INFO(ss.str());

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
		case MessageType::TimeRequest:
		{
			const TimeRequestMessage& timeRequestMessage = static_cast<const TimeRequestMessage&>(message);
			ProcessTimeRequest(timeRequestMessage, address);
			break;
		}
		case MessageType::InGame:
		{
			const InGameMessage& inGameMessage = static_cast<const InGameMessage&>(message);
			ProcessInGame(inGameMessage, address);
			break;
		}
		default:
			Common::LOG_WARNING("Invalid Message type, ignoring it...");
			break;
		}
	}

	void Server::ProcessConnectionRequest(const ConnectionRequestMessage& message, const Address& address)
	{
		std::stringstream ss;
		ss << "Processing connection request from [IP: " << address.GetIP() << ", Port: " << address.GetPort() << "]";
		Common::LOG_INFO(ss.str());

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
				_pendingConnections.emplace_back(address);
				pendingConnectionIndex = _pendingConnections.size() - 1;
				_pendingConnections[pendingConnectionIndex].SetClientSalt(clientSalt);
				_pendingConnections[pendingConnectionIndex].SetServerSalt(GenerateServerSalt());

				std::stringstream ss;
				ss << "Creating a pending connection entry. Client salt: " << clientSalt << " Server salt: " << _pendingConnections[pendingConnectionIndex].GetServerSalt();
				Common::LOG_INFO(ss.str());
			}

			CreateConnectionChallengeMessage(address, pendingConnectionIndex);
		}
		else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
		{
			//int connectedClientIndex = FindExistingClientIndex(address);
			RemotePeer* remotePeer = GetRemotePeerFromAddress(address);
			CreateConnectionApprovedMessage(*remotePeer);
			Common::LOG_INFO("The client is already connected, sending connection approved...");
		}
		else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
		{
			SendConnectionDeniedPacket(address);
			Common::LOG_WARNING("All available connection slots are full. Denying incoming connection...");
		}
	}

	void Server::CreateDisconnectionMessage(RemotePeer& remotePeer)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr<Message>message = messageFactory.LendMessage(MessageType::Disconnection);
		if (message == nullptr)
		{
			Common::LOG_ERROR("Can't create new Disconnection Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<DisconnectionMessage> disconnectionMessage(static_cast<DisconnectionMessage*>(message.release()));

		disconnectionMessage->prefix = remotePeer.GetDataPrefix();
		remotePeer.AddMessage(std::move(disconnectionMessage));

		Common::LOG_INFO("Disconnection message created.");
	}

	void Server::CreateTimeResponseMessage(RemotePeer& remotePeer, const TimeRequestMessage& timeRequest)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message(messageFactory.LendMessage(MessageType::TimeResponse));

		std::unique_ptr<TimeResponseMessage> timeResponseMessage(static_cast<TimeResponseMessage*>(message.release()));
		timeResponseMessage->SetOrdered(true);
		timeResponseMessage->remoteTime = timeRequest.remoteTime;

		TimeClock& timeClock = TimeClock::GetInstance();
		timeResponseMessage->serverTime = timeClock.GetLocalTimeMilliseconds();

		//Find remote client
		remotePeer.AddMessage(std::move(timeResponseMessage));
	}

	void Server::CreateInGameResponseMessage(RemotePeer& remotePeer, uint64_t data)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message>message = messageFactory.LendMessage(MessageType::InGameResponse);
		if (message == nullptr)
		{
			Common::LOG_ERROR("Can't create new in game response Message because the MessageFactory has returned a null message");
			return;
		}

		message->SetReliability(true);
		message->SetOrdered(true);

		std::unique_ptr<InGameResponseMessage> inGameResponseMessage(static_cast<InGameResponseMessage*>(message.release()));

		inGameResponseMessage->data = data;
		remotePeer.AddMessage(std::move(inGameResponseMessage));

		Common::LOG_INFO("In game response message created.");
	}

	void Server::CreateConnectionChallengeMessage(const Address& address, int pendingConnectionIndex)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionChallenge);
		if (message == nullptr)
		{
			Common::LOG_ERROR("Can't create new Connection Challenge Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<ConnectionChallengeMessage> connectionChallengePacket(static_cast<ConnectionChallengeMessage*>(message.release()));
		connectionChallengePacket->clientSalt = _pendingConnections[pendingConnectionIndex].GetClientSalt();
		connectionChallengePacket->serverSalt = _pendingConnections[pendingConnectionIndex].GetServerSalt();
		_pendingConnections[pendingConnectionIndex].AddMessage(std::move(connectionChallengePacket));

		Common::LOG_INFO("Connection challenge message created.");
	}

	void Server::SendConnectionDeniedPacket(const Address& address) const
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionDenied);
		NetworkPacket packet = NetworkPacket();
		packet.AddMessage(std::move(message));

		Common::LOG_INFO("Sending connection denied...");
		SendPacketToAddress(packet, address);

		while (packet.GetNumberOfMessages() > 0)
		{
			std::unique_ptr<Message> messageToReturn = packet.GetMessages();
			messageFactory.ReleaseMessage(std::move(messageToReturn));
		}
	}

	void Server::ProcessConnectionChallengeResponse(const ConnectionChallengeResponseMessage& message, const Address& address)
	{
		std::stringstream ss;
		ss << "Processing connection challenge response from [IP: " << address.GetIP() << ", Port: " << address.GetPort() << "]";
		Common::LOG_INFO(ss.str());

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
				Common::LOG_INFO("Connection denied due to not pending connection found.");
				SendConnectionDeniedPacket(address);
			}
			else
			{
				//Create remote client
				int availableClientSlot = FindFreeRemotePeerSlot();
				AddNewRemotePeer(availableClientSlot, address, dataPrefix);

				//Delete pending connection since we have accepted
				_pendingConnections.erase(_pendingConnections.begin() + pendingConnectionIndex);

				//Send connection approved packet
				CreateConnectionApprovedMessage(_remotePeers[availableClientSlot]);
				Common::LOG_INFO("Connection approved");
			}
		}
		else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
		{
			//Find remote client
			RemotePeer* remotePeer = GetRemotePeerFromAddress(address);

			//Check if data prefix match
			if (remotePeer->GetDataPrefix() != dataPrefix)
			{
				return;
			}

			CreateConnectionApprovedMessage(*remotePeer);
		}
		else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
		{
			SendConnectionDeniedPacket(address);
		}
	}

	//REFACTOR THIS METHOD
	void Server::ProcessTimeRequest(const TimeRequestMessage& message, const Address& address)
	{
		Common::LOG_INFO("PROCESSING TIME REQUEST");
		RemotePeer* remotePeer = GetRemotePeerFromAddress(address);
		CreateTimeResponseMessage(*remotePeer, message);
	}

	void Server::ProcessInGame(const InGameMessage& message, const Address& address)
	{
		std::stringstream ss;
		ss << "InGame ID: " << message.data;
		Common::LOG_INFO(ss.str());

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

		int availableClientSlot = FindFreeRemotePeerSlot();
		if (availableClientSlot == -1)
		{
			return -1;
		}

		return 0;
	}

	void Server::AddNewRemotePeer(int remotePeerSlotIndex, const Address& address, uint64_t dataPrefix)
	{
		_remotePeerSlots[remotePeerSlotIndex] = true;
		_remotePeers[remotePeerSlotIndex].Connect(address.GetInfo(), _nextAssignedRemotePeerID, REMOTE_CLIENT_INACTIVITY_TIME, dataPrefix);
		++_nextAssignedRemotePeerID;
	}

	void Server::CreateConnectionApprovedMessage(RemotePeer& remotePeer)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionAccepted);
		if (message == nullptr)
		{
			Common::LOG_ERROR("Can't create new Connection Accepted Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<ConnectionAcceptedMessage> connectionAcceptedPacket(static_cast<ConnectionAcceptedMessage*>(message.release()));
		connectionAcceptedPacket->prefix = remotePeer.GetDataPrefix();
		connectionAcceptedPacket->clientIndexAssigned = remotePeer.GetClientIndex();
		remotePeer.AddMessage(std::move(connectionAcceptedPacket));
	}

	void Server::SendPacketToRemotePeer(const RemotePeer& remotePeer, const NetworkPacket& packet) const
	{
		SendPacketToAddress(packet, remotePeer.GetAddress());
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
}
