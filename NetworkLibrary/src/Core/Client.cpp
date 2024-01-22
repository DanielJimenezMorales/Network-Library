#include <sstream>

#include "Client.h"
#include "Message.h"
#include "Logger.h"
#include "NetworkPacket.h"
#include "MessageFactory.h"
#include "RemotePeer.h"
#include "PendingConnection.h"

Client::Client(float serverMaxInactivityTimeout) : Peer(PeerType::ClientMode, 1, 1024, 1024),
			_serverMaxInactivityTimeout(serverMaxInactivityTimeout),
			_serverInactivityTimeLeft(serverMaxInactivityTimeout),
			_saltNumber(0),
			_dataPrefix(0),
			_serverAddress("127.0.0.1", 54000)
{
}

Client::~Client()
{
}

bool Client::StartConcrete()
{
	_currentState = ClientState::SendingConnectionRequest;
	_serverInactivityTimeLeft = _serverMaxInactivityTimeout;

	GenerateClientSaltNumber();
	_pendingConnections.push_back(PendingConnection(_serverAddress));

	LOG_INFO("Client started succesfully!");

	return true;
}

void Client::GenerateClientSaltNumber()
{
	//TODO Change this for a better generator. rand is not generating a full 64bit integer since its maximum is roughly 32767. I have tried to use mt19937_64 but I think I get a conflict with winsocks and std::uniform_int_distribution
	srand(time(NULL));
	_saltNumber = rand();
}

void Client::ProcessMessage(const Message& message, const Address& address)
{
	MessageType messageType = message.GetHeader().type;

	switch (messageType)
	{
	case MessageType::ConnectionChallenge:
		if (_currentState == ClientState::SendingConnectionRequest || _currentState == ClientState::SendingConnectionChallengeResponse)
		{
			const ConnectionChallengeMessage& connectionChallengeMessage = static_cast<const ConnectionChallengeMessage&>(message);
			ProcessConnectionChallenge(connectionChallengeMessage);
		}
		break;
	case MessageType::ConnectionAccepted:
		if (_currentState == ClientState::SendingConnectionChallengeResponse)
		{
			const ConnectionAcceptedMessage& connectionAcceptedMessage = static_cast<const ConnectionAcceptedMessage&>(message);
			ProcessConnectionRequestAccepted(connectionAcceptedMessage);
		}
		break;
	case MessageType::ConnectionDenied:
		if (_currentState == ClientState::SendingConnectionChallengeResponse || _currentState == ClientState::SendingConnectionRequest)
		{
			const ConnectionDeniedMessage& connectionDeniedMessage = static_cast<const ConnectionDeniedMessage&>(message);
			ProcessConnectionRequestDenied(connectionDeniedMessage);
		}
		break;
	case MessageType::Disconnection:
		if (_currentState == ClientState::Connected)
		{
			const DisconnectionMessage& disconnectionMessage = static_cast<const DisconnectionMessage&>(message);
			ProcessDisconnection(disconnectionMessage);
		}
		break;
	default:
		LOG_WARNING("Invalid Message type, ignoring it...");
		break;
	}
}

void Client::TickConcrete(float elapsedTime)
{
	if (_currentState == ClientState::SendingConnectionRequest || _currentState == ClientState::SendingConnectionChallengeResponse)
	{
		CreateConnectionRequestMessage();
	}

	if (_currentState != ClientState::Disconnected)
	{
		_serverInactivityTimeLeft -= elapsedTime;

		if (_serverInactivityTimeLeft <= 0.f)
		{
			LOG_INFO("Server inactivity timeout reached. Disconnecting client...");
			_serverInactivityTimeLeft = 0.f;
			_currentState = ClientState::Disconnected;
		}
	}
}

void Client::DisconnectRemotePeerConcrete(RemotePeer& remotePeer)
{
}

bool Client::StopConcrete()
{
	return true;
}

void Client::ProcessConnectionChallenge(const ConnectionChallengeMessage& message)
{
	LOG_INFO("Challenge packet received from server");

	uint64_t clientSalt = message.clientSalt;
	uint64_t serverSalt = message.serverSalt;
	if (_saltNumber != clientSalt)
	{
		LOG_WARNING("The generated salt number does not match the server's challenge client salt number. Aborting operation");
		return;
	}

	_dataPrefix = clientSalt ^ serverSalt; //XOR operation to create the data prefix for all packects from now on

	_currentState = ClientState::SendingConnectionChallengeResponse;

	CreateConnectionChallengeResponse();

	LOG_INFO("Sending challenge response packet to server...");
}

void Client::ProcessConnectionRequestAccepted(const ConnectionAcceptedMessage& message)
{
	uint64_t dataPrefix = message.prefix;
	if (dataPrefix != _dataPrefix)
	{
		LOG_WARNING("Packet prefix does not match. Skipping packet...");
		return;
	}

	_pendingConnections.erase(_pendingConnections.begin());

	_remotePeerSlots[0] = true;
	_remotePeers[0].Connect(_serverAddress.GetInfo(), 0, 5, dataPrefix);

	_clientIndex = message.clientIndexAssigned;
	_currentState = ClientState::Connected;
	LOG_INFO("Connection accepted!");
}

void Client::ProcessConnectionRequestDenied(const ConnectionDeniedMessage& message)
{
	_currentState = ClientState::Disconnected;
	LOG_INFO("Connection denied");
}

void Client::ProcessDisconnection(const DisconnectionMessage& message)
{
	uint64_t dataPrefix = message.prefix;
	if (dataPrefix != _dataPrefix)
	{
		LOG_WARNING("Packet prefix does not match. Skipping packet...");
		return;
	}

	_currentState = ClientState::Disconnected();
	LOG_INFO("Disconnection message received from server. Disconnecting...");
}

void Client::CreateConnectionRequestMessage()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	Message* message = messageFactory->LendMessage(MessageType::ConnectionRequest);
	if (message == nullptr)
	{
		LOG_ERROR("Can't create new Connection Request Message because the MessageFactory has returned a null message");
		return;
	}

	ConnectionRequestMessage* connectionRequestMessage = static_cast<ConnectionRequestMessage*>(message);

	connectionRequestMessage->clientSalt = _saltNumber;

	_pendingConnections[0].AddMessage(connectionRequestMessage);

	LOG_INFO("Connection request created.");
}

void Client::CreateConnectionChallengeResponse()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	Message* message = messageFactory->LendMessage(MessageType::ConnectionChallengeResponse);
	if (message == nullptr)
	{
		LOG_ERROR("Can't create new Connection Challenge Response Message because the MessageFactory has returned a null message");
		return;
	}

	ConnectionChallengeResponseMessage* connectionChallengeResponsePacket = static_cast<ConnectionChallengeResponseMessage*>(message);
	connectionChallengeResponsePacket->prefix = _dataPrefix;

	_pendingConnections[0].AddMessage(connectionChallengeResponsePacket);
}