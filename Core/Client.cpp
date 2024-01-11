#include "Client.h"
//#include "NetworkPacket.h"
#include "Message.h"
#include "Buffer.h"
#include "Logger.h"
#include <NetworkPacket.h>

Client::Client(float serverMaxInactivityTimeout) : 
			_serverMaxInactivityTimeout(serverMaxInactivityTimeout),
			_serverInactivityTimeLeft(serverMaxInactivityTimeout),
			_saltNumber(0),
			_dataPrefix(0)
{
	_serverAddress = Address("127.0.0.1", 54000);
}

Client::~Client()
{
}

int Client::Start()
{
	LOG_INFO("Starting client...");

	int iResult = 0;
	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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
		return iResult;
	}

	bind(_socket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);

	_currentState = ClientState::SendingConnectionRequest;
	_serverInactivityTimeLeft = _serverMaxInactivityTimeout;

	GenerateClientSaltNumber();
	SendConnectionRequestPacket();

	LOG_INFO("Client started succesfully!");
	return iResult;
}

void Client::Tick(float elapsedTime)
{
	int iResult = 0;

	if (_currentState == ClientState::SendingConnectionRequest || _currentState == ClientState::SendingConnectionChallengeResponse)
	{
		SendConnectionRequestPacket();
	}

	if (IsThereNewDataToProcess())
	{
		ProcessReceivedData();
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

void Client::GenerateClientSaltNumber()
{
	//TODO Change this for a better generator. rand is not generating a full 64bit integer since its maximum is roughly 32767. I have tried to use mt19937_64 but I think I get a conflict with winsocks and std::uniform_int_distribution
	srand(time(NULL));
	_saltNumber = rand();
}

bool Client::IsThereNewDataToProcess() const
{
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(_socket, &readSet);

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

void Client::ProcessReceivedData()
{
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);

	int size = 1024;
	uint8_t* data = new uint8_t[size];

	int bytesIn = recvfrom(_socket, (char*)data, size, 0, (sockaddr*)&client, &clientLength);
	if (bytesIn == SOCKET_ERROR)
	{		
		LOG_ERROR("Error while receiving a message, error code " + WSAGetLastError());
	}

	Buffer* buffer = new Buffer(data, bytesIn);
	Address address = Address(client);
	ProcessDatagram(*buffer, address);

	delete buffer;
	buffer = nullptr;
}

void Client::ProcessDatagram(Buffer& buffer, const Address& address)
{
	//AHORA ESTO RECIBE UN NETWORK PACKET CON MENSAJES DENTRO
	NetworkPacket packet = NetworkPacket();
	packet.Read(buffer);

	std::vector<Message*>::const_iterator constIterator = packet.GetMessages();
	unsigned int numberOfMessagesInPacket = packet.GetNumberOfMessages();
	MessageType messageType;
	Message* message = nullptr;
	for (unsigned int i = 0; i < numberOfMessagesInPacket; ++i)
	{
		message = *(constIterator+i);
		messageType = message->header.type;

		switch (messageType)
		{
		case MessageType::ConnectionChallenge:
			if (_currentState == ClientState::SendingConnectionRequest || _currentState == ClientState::SendingConnectionChallengeResponse)
			{
				//ProcessConnectionChallenge(buffer);
				ProcessConnectionChallenge(*static_cast<ConnectionChallengeMessage*>(message));
			}
			break;
		case MessageType::ConnectionAccepted:
			if (_currentState == ClientState::SendingConnectionChallengeResponse)
			{
				//ProcessConnectionRequestAccepted(buffer);
				ProcessConnectionRequestAccepted(*static_cast<ConnectionAcceptedMessage*>(message));
			}
			break;
		case MessageType::ConnectionDenied:
			if (_currentState == ClientState::SendingConnectionChallengeResponse || _currentState == ClientState::SendingConnectionRequest)
			{
				//ProcessConnectionRequestDenied();
				ProcessConnectionRequestDenied(*static_cast<ConnectionDeniedMessage*>(message));
			}
			break;
		case MessageType::Disconnection:
			if (_currentState == ClientState::Connected)
			{
				//ProcessDisconnection(buffer);
				ProcessDisconnection(*static_cast<DisconnectionMessage*>(message));
			}
			break;
		default:
			LOG_WARNING("Invalid datagram, ignoring it...");
			break;
		}
	}


	/*uint8_t packetType = buffer.ReadByte();

	switch (packetType)
	{
	case MessageType::ConnectionChallenge:
		if (_currentState == ClientState::SendingConnectionRequest || _currentState == ClientState::SendingConnectionChallengeResponse)
		{
			ProcessConnectionChallenge(buffer);
		}
		break;
	case MessageType::ConnectionAccepted:
		if (_currentState == ClientState::SendingConnectionChallengeResponse)
		{
			ProcessConnectionRequestAccepted(buffer);
		}
		break;
	case MessageType::ConnectionDenied:
		if (_currentState == ClientState::SendingConnectionChallengeResponse || _currentState == ClientState::SendingConnectionRequest)
		{
			ProcessConnectionRequestDenied();
		}
		break;
	case MessageType::Disconnection:
		if (_currentState == ClientState::Connected)
		{
			ProcessDisconnection(buffer);
		}
		break;
	default:
		LOG_WARNING("Invalid datagram, ignoring it...");
		break;
	}*/
}

void Client::ProcessConnectionChallenge(Buffer& buffer)
{
	LOG_INFO("Challenge packet received from server");

	uint64_t clientSalt = buffer.ReadLong();
	uint64_t serverSalt = buffer.ReadLong();
	if (_saltNumber != clientSalt)
	{
		LOG_WARNING("The generated salt number does not match the server's challenge client salt number. Aborting operation");
		return;
	}

	_dataPrefix = clientSalt ^ serverSalt; //XOR operation to create the data prefix for all packects from now on

	_currentState = ClientState::SendingConnectionChallengeResponse;

	ConnectionChallengeResponseMessage connectionChallengeResponsePacket;
	connectionChallengeResponsePacket.prefix = _dataPrefix;
	Buffer* challengeResponseBuffer = new Buffer(sizeof(connectionChallengeResponsePacket));
	connectionChallengeResponsePacket.Write(*challengeResponseBuffer);

	LOG_INFO("Sending challenge response packet to server...");
	SendPacketToServer(*challengeResponseBuffer);

	delete challengeResponseBuffer;
	challengeResponseBuffer = nullptr;
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

	ConnectionChallengeResponseMessage connectionChallengeResponsePacket;
	connectionChallengeResponsePacket.prefix = _dataPrefix;
	Buffer* challengeResponseBuffer = new Buffer(sizeof(connectionChallengeResponsePacket));
	connectionChallengeResponsePacket.Write(*challengeResponseBuffer);

	LOG_INFO("Sending challenge response packet to server...");
	SendPacketToServer(*challengeResponseBuffer);

	delete challengeResponseBuffer;
	challengeResponseBuffer = nullptr;
}

void Client::ProcessConnectionRequestAccepted(Buffer& buffer)
{
	uint64_t dataPrefix = buffer.ReadLong();
	if (dataPrefix != _dataPrefix)
	{
		LOG_WARNING("Packet prefix does not match. Skipping packet...");
		return;
	}

	_clientIndex = buffer.ReadShort();
	_currentState = ClientState::Connected;
	LOG_INFO("Connection accepted!");
}

void Client::ProcessConnectionRequestAccepted(const ConnectionAcceptedMessage& message)
{
	uint64_t dataPrefix = message.prefix;
	if (dataPrefix != _dataPrefix)
	{
		LOG_WARNING("Packet prefix does not match. Skipping packet...");
		return;
	}

	_clientIndex = message.clientIndexAssigned;
	_currentState = ClientState::Connected;
	LOG_INFO("Connection accepted!");
}

void Client::ProcessConnectionRequestDenied()
{
	_currentState = ClientState::Disconnected;
	LOG_INFO("Connection denied");
}

void Client::ProcessConnectionRequestDenied(const ConnectionDeniedMessage& message)
{
	_currentState = ClientState::Disconnected;
	LOG_INFO("Connection denied");
}

void Client::ProcessDisconnection(Buffer& buffer)
{
	uint64_t dataPrefix = buffer.ReadLong();
	if (dataPrefix != _dataPrefix)
	{
		LOG_WARNING("Packet prefix does not match. Skipping packet...");
		return;
	}

	_currentState = ClientState::Disconnected();
	LOG_INFO("Disconnection message received from server. Disconnecting...");
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

void Client::SendConnectionRequestPacket()
{
	LOG_INFO("Sending connection request to server...");
	ConnectionRequestMessage connectionRequestPacket = ConnectionRequestMessage();
	connectionRequestPacket.clientSalt = _saltNumber;
	int connectionRequestPacketSize = sizeof(connectionRequestPacket);
	Buffer* buffer = new Buffer(connectionRequestPacketSize);
	connectionRequestPacket.Write(*buffer);
	SendPacketToServer(*buffer);

	delete buffer;
	buffer = nullptr;
}

void Client::SendPacketToServer(const Buffer& buffer) const
{
	int bytesSent = sendto(_socket, (char*)buffer.GetData(), buffer.GetSize(), 0, (sockaddr*)&_serverAddress.GetInfo(), sizeof(_serverAddress.GetInfo()));
	if (bytesSent == SOCKET_ERROR)
	{
		int iResult = WSAGetLastError();
		LOG_ERROR("Error while sending datagram to server, error code " + iResult);
	}
}

int Client::Stop()
{
	LOG_INFO("Stopping client...");

	int iResult = 0;
	iResult = closesocket(_socket);
	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		LOG_ERROR("Error while closing listen socket, error code " + iResult);
	}

	return iResult;
}