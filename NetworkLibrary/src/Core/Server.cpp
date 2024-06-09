#include <cassert>
#include <memory>

#include "Server.h"
#include "Logger.h"
#include "NetworkPacket.h"
#include "Message.h"
#include "MessageFactory.h"
#include "TimeClock.h"

#define SERVER_PORT 54000

namespace NetLib
{
	Server::Server(int maxConnections) : Peer(PeerType::ServerMode, maxConnections, 1024, 1024)
	{

	}

	uint32_t Server::CreateNetworkEntity(uint32_t entityType, float posX, float posY)
	{
		if (GetConnectionState() != PeerConnectionState::PCS_Connected)
		{
			LOG_WARNING("Can't create Network entity of type %d because the server is not connected.", static_cast<int>(entityType));
			return 0;
		}

		return _replicationManager.CreateNetworkEntity(entityType, posX, posY);
	}

	void Server::DestroyNetworkEntity(uint32_t entityId)
	{
		if (GetConnectionState() != PeerConnectionState::PCS_Connected)
		{
			LOG_WARNING("Can't destroy Network entity with ID: %d because the server is not connected.", static_cast<int>(entityId));
			return;
		}

		_replicationManager.RemoveNetworkEntity(entityId);
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

		ExecuteOnLocalPeerConnect();
		return true;
	}

	void Server::TickConcrete(float elapsedTime)
	{
		TickReplication();
	}

	uint64_t Server::GenerateServerSalt() const
	{
		//TODO Change this in order to get another random generator that generates 64bit numbers
		srand(time(NULL) + 3589);
		uint64_t serverSalt = rand();
		return serverSalt;
	}

	void Server::ProcessMessageFromPeer(const Message& message, RemotePeer& remotePeer)
	{
		MessageType messageType = message.GetHeader().type;
		LOG_INFO("Me proceso type %d", static_cast<int>(messageType));

		switch (messageType)
		{
		case MessageType::ConnectionRequest:
		{
			const ConnectionRequestMessage& connectionRequestMessage = static_cast<const ConnectionRequestMessage&>(message);
			ProcessConnectionRequest(connectionRequestMessage, remotePeer.GetAddress());
			break;
		}
		case MessageType::ConnectionChallengeResponse:
		{
			const ConnectionChallengeResponseMessage& connectionChallengeResponseMessage = static_cast<const ConnectionChallengeResponseMessage&>(message);
			ProcessConnectionChallengeResponse(connectionChallengeResponseMessage, remotePeer);
			break;
		}
		case MessageType::TimeRequest:
		{
			const TimeRequestMessage& timeRequestMessage = static_cast<const TimeRequestMessage&>(message);
			ProcessTimeRequest(timeRequestMessage, remotePeer);
			break;
		}
		case MessageType::Disconnection:
		{
			const DisconnectionMessage& disconnectionMessage = static_cast<const DisconnectionMessage&>(message);
			ProcessDisconnection(disconnectionMessage, remotePeer);
			break;
		}
		case MessageType::InGame:
		{
			const InGameMessage& inGameMessage = static_cast<const InGameMessage&>(message);
			ProcessInGame(inGameMessage, remotePeer);
			break;
		}
		default:
			LOG_WARNING("Invalid Message type, ignoring it...");
			break;
		}
	}

	void Server::ProcessMessageFromUnknownPeer(const Message& message, const Address& address)
	{
		if (message.GetHeader().type == MessageType::ConnectionRequest)
		{
			const ConnectionRequestMessage& connectionRequestMessage = static_cast<const ConnectionRequestMessage&>(message);
			ProcessConnectionRequest(connectionRequestMessage, address);
		}
		else
		{
			LOG_WARNING("Server only process Connection request messages from unknown peers. Any other type of message will be discarded.");
		}
	}

	void Server::ProcessConnectionRequest(const ConnectionRequestMessage& message, const Address& address)
	{
		LOG_INFO("Processing connection request from [IP: %s, Port: %hu] with salt number %d", address.GetIP().c_str(), (int)address.GetPort(), message.clientSalt);

		RemotePeersHandlerResult isAbleToConnectResult = _remotePeersHandler.IsRemotePeerAbleToConnect(address);

		if (isAbleToConnectResult == RemotePeersHandlerResult::RPH_SUCCESS)//If there is green light keep with the connection pipeline.
		{
			uint64_t clientSalt = message.clientSalt;
			uint64_t serverSalt = GenerateServerSalt();
			AddRemotePeer(address, _nextAssignedRemotePeerID, clientSalt, serverSalt);
			++_nextAssignedRemotePeerID;

			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);
			CreateConnectionChallengeMessage(*remotePeer);
		}
		else if (isAbleToConnectResult == RemotePeersHandlerResult::RPH_ALREADYEXIST)//If the client is already connected just send a connection approved message
		{
			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress(address);

			RemotePeerState remotePeerState = remotePeer->GeturrentState();
			if (remotePeerState == RemotePeerState::Connected)
			{
				CreateConnectionApprovedMessage(*remotePeer);
				LOG_INFO("The client is already connected, sending connection approved...");
			}
			else if (remotePeerState == RemotePeerState::Connecting)
			{
				CreateConnectionChallengeMessage(*remotePeer);
				LOG_INFO("The client is already trying to connect, sending connection challenge...");
			}
		}
		else if (isAbleToConnectResult == RemotePeersHandlerResult::RPH_FULL)//If all the client slots are full deny the connection
		{
			SendConnectionDeniedPacket(address, ConnectionFailedReasonType::CFR_SERVER_FULL);
			LOG_WARNING("All available connection slots are full. Denying incoming connection...");
		}
	}

	void Server::CreateDisconnectionMessage(RemotePeer& remotePeer)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr<Message>message = messageFactory.LendMessage(MessageType::Disconnection);
		if (message == nullptr)
		{
			LOG_ERROR("Can't create new Disconnection Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<DisconnectionMessage> disconnectionMessage(static_cast<DisconnectionMessage*>(message.release()));

		disconnectionMessage->prefix = remotePeer.GetDataPrefix();
		remotePeer.AddMessage(std::move(disconnectionMessage));

		LOG_INFO("Disconnection message created.");
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
			LOG_ERROR("Can't create new in game response Message because the MessageFactory has returned a null message");
			return;
		}

		message->SetReliability(true);
		message->SetOrdered(true);

		std::unique_ptr<InGameResponseMessage> inGameResponseMessage(static_cast<InGameResponseMessage*>(message.release()));

		inGameResponseMessage->data = data;
		remotePeer.AddMessage(std::move(inGameResponseMessage));

		LOG_INFO("In game response message created.");
	}

	void Server::CreateConnectionChallengeMessage(RemotePeer& remotePeer)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionChallenge);
		if (message == nullptr)
		{
			LOG_ERROR("Can't create new Connection Challenge Message because the MessageFactory has returned a null message");
			return;
		}

		std::unique_ptr<ConnectionChallengeMessage> connectionChallengePacket(static_cast<ConnectionChallengeMessage*>(message.release()));
		connectionChallengePacket->clientSalt = remotePeer.GetClientSalt();
		connectionChallengePacket->serverSalt = remotePeer.GetServerSalt();
		remotePeer.AddMessage(std::move(connectionChallengePacket));

		LOG_INFO("Connection challenge message created.");
	}

	void Server::SendConnectionDeniedPacket(const Address& address, ConnectionFailedReasonType reason) const
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionDenied);
		std::unique_ptr<ConnectionDeniedMessage> connectionDeniedMessage(static_cast<ConnectionDeniedMessage*>(message.release()));
		connectionDeniedMessage->reason = reason;

		NetworkPacket packet = NetworkPacket();
		packet.AddMessage(std::move(connectionDeniedMessage));

		LOG_INFO("Sending connection denied...");
		SendPacketToAddress(packet, address);

		while (packet.GetNumberOfMessages() > 0)
		{
			std::unique_ptr<Message> messageToReturn = packet.GetMessages();
			messageFactory.ReleaseMessage(std::move(messageToReturn));
		}
	}

	void Server::ProcessConnectionChallengeResponse(const ConnectionChallengeResponseMessage& message, RemotePeer& remotePeer)
	{
		LOG_INFO("Processing connection challenge response from [IP: %s , Port: %hu]", remotePeer.GetAddress().GetIP(), remotePeer.GetAddress().GetPort());

		if (remotePeer.GeturrentState() == RemotePeerState::Connected)
		{
			LOG_INFO("The remote peer is already connected. Sending connection approved...");
			CreateConnectionApprovedMessage(remotePeer);
			return;
		}

		uint64_t dataPrefix = message.prefix;

		if (remotePeer.GetDataPrefix() == dataPrefix)
		{
			ConnectRemotePeer(remotePeer);

			//Send connection approved packet
			CreateConnectionApprovedMessage(remotePeer);
			LOG_INFO("Connection approved");
		}
		else
		{
			LOG_INFO("Connection denied due to not wrong data prefix");
			SendConnectionDeniedPacket(remotePeer.GetAddress(), ConnectionFailedReasonType::CFR_UNKNOWN);

			StartDisconnectingRemotePeer(remotePeer.GetClientIndex(), false, ConnectionFailedReasonType::CFR_UNKNOWN);
		}
	}

	//TODO REFACTOR THIS METHOD
	void Server::ProcessTimeRequest(const TimeRequestMessage& message, RemotePeer& remotePeer)
	{
		LOG_INFO("PROCESSING TIME REQUEST");
		CreateTimeResponseMessage(remotePeer, message);
	}

	void Server::ProcessInGame(const InGameMessage& message, RemotePeer& remotePeer)
	{
		LOG_INFO("InGame ID: %llu", message.data);

		CreateInGameResponseMessage(remotePeer, message.data);
	}

	void Server::ProcessDisconnection(const DisconnectionMessage& message, RemotePeer& remotePeer)
	{
		uint64_t dataPrefix = message.prefix;
		if (dataPrefix != remotePeer.GetDataPrefix())
		{
			LOG_WARNING("Packet prefix does not match. Skipping message...");
			return;
		}

		LOG_INFO("Disconnection message received from remote peer with reason code equal to %hhu. Disconnecting remove peer...", message.reason);

		StartDisconnectingRemotePeer(remotePeer.GetClientIndex(), false, ConnectionFailedReasonType::CFR_UNKNOWN);
	}

	void Server::CreateConnectionApprovedMessage(RemotePeer& remotePeer)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::ConnectionAccepted);
		if (message == nullptr)
		{
			LOG_ERROR("Can't create new Connection Accepted Message because the MessageFactory has returned a null message");
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

	void Server::TickReplication()
	{
		_replicationManager.Server_ReplicateWorldState();

		MessageFactory& messageFactory = MessageFactory::GetInstance();

		while (_replicationManager.ArePendingReplicationMessages())
		{
			const ReplicationMessage* pendingReplicationMessage = _replicationManager.GetPendingReplicationMessage();

			auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
			auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

			for (; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt)
			{
				std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::Replication);
				std::unique_ptr<ReplicationMessage> replicationMessage(static_cast<ReplicationMessage*>(message.release()));

				replicationMessage->SetOrdered(pendingReplicationMessage->GetHeader().isOrdered);
				replicationMessage->SetReliability(pendingReplicationMessage->GetHeader().isReliable);
				replicationMessage->replicationAction = pendingReplicationMessage->replicationAction;
				replicationMessage->networkEntityId = pendingReplicationMessage->networkEntityId;
				replicationMessage->replicatedClassId = pendingReplicationMessage->replicatedClassId;
				replicationMessage->dataSize = pendingReplicationMessage->dataSize;
				if (replicationMessage->dataSize > 0)
				{
					//TODO Figure out if I can improve this. So far, for large snapshot updates data this can become heavy and slow. Can I avoid the copy somehow?
					uint8_t* data = new uint8_t[replicationMessage->dataSize];
					std::memcpy(data, pendingReplicationMessage->data, replicationMessage->dataSize);
					replicationMessage->data = data;
				}

				(*validRemotePeersIt)->AddMessage(std::move(replicationMessage));
			}
		}

		_replicationManager.ClearSentReplicationMessages();
	}

	bool Server::StopConcrete()
	{
		return true;
	}
}
