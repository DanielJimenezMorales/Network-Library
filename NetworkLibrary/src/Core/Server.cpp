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
		case MessageType::Disconnection:
		{
			const DisconnectionMessage& disconnectionMessage = static_cast<const DisconnectionMessage&>(message);
			ProcessDisconnection(disconnectionMessage, address);
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
		ss << "Processing connection request from [IP: " << address.GetIP() << ", Port: " << address.GetPort() << "] with salt number " << message.clientSalt;
		Common::LOG_INFO(ss.str());

		int isAbleToConnectResult = IsRemotePeerAbleToConnect(address);

		if (isAbleToConnectResult == 0)//If there is green light keep with the connection pipeline.
		{
			uint64_t clientSalt = message.clientSalt;
			PendingConnection* pendingConnection = nullptr;
			for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
			{
				if (_pendingConnectionSlots[i])
				{
					if (_pendingConnections[i].IsAddressEqual(address) && _pendingConnections[i].GetClientSalt() == clientSalt)
					{
						pendingConnection = GetPendingConnectionFromIndex(i);
						break;
					}
				}
			}

			if (pendingConnection == nullptr) //If no pending connection was found create one!
			{
				int pendingConnectionIndex = AddPendingConnection(address, 1.f);
				pendingConnection = GetPendingConnectionFromIndex(pendingConnectionIndex);
				pendingConnection->SetClientSalt(clientSalt);
				pendingConnection->SetServerSalt(GenerateServerSalt());

				std::stringstream ss;
				ss << "Creating a pending connection entry. Client salt: " << clientSalt << " Server salt: " << pendingConnection->GetServerSalt();
				Common::LOG_INFO(ss.str());
			}

			CreateConnectionChallengeMessage(address, *pendingConnection);
		}
		else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
		{
			//int connectedClientIndex = FindExistingClientIndex(address);
			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);
			CreateConnectionApprovedMessage(*remotePeer);
			Common::LOG_INFO("The client is already connected, sending connection approved...");
		}
		else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
		{
			SendConnectionDeniedPacket(address, ConnectionFailedReasonType::CFR_SERVER_FULL);
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

	void Server::CreateConnectionChallengeMessage(const Address& address, PendingConnection& pendingConnection)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionChallenge);
		if (message == nullptr)
		{
			Common::LOG_ERROR("Can't create new Connection Challenge Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<ConnectionChallengeMessage> connectionChallengePacket(static_cast<ConnectionChallengeMessage*>(message.release()));
		connectionChallengePacket->clientSalt = pendingConnection.GetClientSalt();
		connectionChallengePacket->serverSalt = pendingConnection.GetServerSalt();
		pendingConnection.AddMessage(std::move(connectionChallengePacket));

		Common::LOG_INFO("Connection challenge message created.");
	}

	void Server::SendConnectionDeniedPacket(const Address& address, ConnectionFailedReasonType reason) const
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionDenied);
		std::unique_ptr<ConnectionDeniedMessage> connectionDeniedMessage(static_cast<ConnectionDeniedMessage*>(message.release()));
		connectionDeniedMessage->reason = reason;

		NetworkPacket packet = NetworkPacket();
		packet.AddMessage(std::move(connectionDeniedMessage));

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

		int isAbleToConnectResult = IsRemotePeerAbleToConnect(address);

		if (isAbleToConnectResult == 0)//If there is green light keep with the connection pipeline.
		{
			//Search for a pending connection that matches the challenge response
			int pendingConnectionIndex = -1;
			for (unsigned int i = 0; i < _pendingConnections.size(); ++i)
			{
				if (_pendingConnectionSlots[i])
				{
					if (_pendingConnections[i].GetPrefix() == dataPrefix && _pendingConnections[i].IsAddressEqual(address))
					{
						pendingConnectionIndex = i;
						break;
					}
				}
			}

			if (pendingConnectionIndex == -1)
			{
				Common::LOG_INFO("Connection denied due to not pending connection found.");
				SendConnectionDeniedPacket(address, ConnectionFailedReasonType::CFR_UNKNOWN);
			}
			else
			{
				//Create remote client
				AddRemotePeer(address, _nextAssignedRemotePeerID, dataPrefix);
				++_nextAssignedRemotePeerID;

				//Delete pending connection since we have accepted
				RemovePendingConnectionAtIndex(pendingConnectionIndex);

				//Send connection approved packet
				RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);
				CreateConnectionApprovedMessage(*remotePeer);
				Common::LOG_INFO("Connection approved");
			}
		}
		else if (isAbleToConnectResult == 1)//If the client is already connected just send a connection approved message
		{
			//Find remote client
			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);

			//Check if data prefix match
			if (remotePeer->GetDataPrefix() != dataPrefix)
			{
				return;
			}

			CreateConnectionApprovedMessage(*remotePeer);
		}
		else if (isAbleToConnectResult == -1)//If all the client slots are full deny the connection
		{
			SendConnectionDeniedPacket(address, ConnectionFailedReasonType::CFR_SERVER_FULL);
		}
	}

	//REFACTOR THIS METHOD
	void Server::ProcessTimeRequest(const TimeRequestMessage& message, const Address& address)
	{
		Common::LOG_INFO("PROCESSING TIME REQUEST");
		RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);
		CreateTimeResponseMessage(*remotePeer, message);
	}

	void Server::ProcessInGame(const InGameMessage& message, const Address& address)
	{
		std::stringstream ss;
		ss << "InGame ID: " << message.data;
		Common::LOG_INFO(ss.str());

		RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);
		assert(remotePeer != nullptr);

		CreateInGameResponseMessage(*remotePeer, message.data);
	}

	void Server::ProcessDisconnection(const DisconnectionMessage& message, const Address& address)
	{
		RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);
		if (remotePeer == nullptr)
		{
			std::stringstream ss;
			ss << "There is no remote peer corresponding with IP: " << address.GetIP();
			Common::LOG_ERROR(ss.str());
			return;
		}

		uint64_t dataPrefix = message.prefix;
		if (dataPrefix != remotePeer->GetDataPrefix())
		{
			Common::LOG_WARNING("Packet prefix does not match. Skipping message...");
			return;
		}

		std::stringstream ss;
		ss << "Disconnection message received from remote peer with reason code equal to " << (int)message.reason << ". Disconnecting remove peer...";
		Common::LOG_INFO(ss.str());

		StartDisconnectingRemotePeer(remotePeer->GetClientIndex(), false, ConnectionFailedReasonType::CFR_UNKNOWN);
	}

	int Server::IsRemotePeerAbleToConnect(const Address& address) const
	{
		if (_remotePeersHandler.IsRemotePeerAlreadyConnected(address))
		{
			return 1;
		}

		int availableClientSlot = _remotePeersHandler.FindFreeRemotePeerSlot();
		if (availableClientSlot == -1)
		{
			return -1;
		}

		return 0;
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

	bool Server::StopConcrete()
	{
		return true;
	}
}
