#pragma once

//Define this before any include in order to be able to use std::numeric_limits<uint64_t>::min() and std::numeric_limits<uint64_t>::max() methods and not getting errors with the ones
//from Windows.h
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <queue>

#include "Address.h"

class Buffer;
class Message;
class ConnectionChallengeMessage;
class ConnectionAcceptedMessage;
class ConnectionDeniedMessage;
class DisconnectionMessage;

enum ClientState
{
	Disconnected = 0,
	Connected = 1,
	SendingConnectionRequest = 2,
	SendingConnectionChallengeResponse = 3,
};

class Client
{
public:
	Client(float serverMaxInactivityTimeout);
	~Client();

	int Start();
	int Stop();

	void Tick(float elapsedTime);

private:
	void GenerateClientSaltNumber();

	bool IsThereNewDataToProcess() const;
	void ProcessReceivedData();
	void ProcessDatagram(Buffer& buffer, const Address& address);
	void ProcessConnectionChallenge(const ConnectionChallengeMessage& message);
	void ProcessConnectionRequestAccepted(const ConnectionAcceptedMessage& message);
	void ProcessConnectionRequestDenied(const ConnectionDeniedMessage& message);
	void ProcessDisconnection(const DisconnectionMessage& message);

	void SendData();
	void CreateConnectionRequestMessage();
	void CreateConnectionChallengeResponse();
	void SendConnectionRequestPacket();
	void SendPacketToServer(const Buffer& buffer) const;

	bool AddMessage(Message* message);
	bool ArePendingMessages() const { return !_pendingMessages.empty(); }
	Message* GetAMessage();

	void FreeSentMessages();

	SOCKET _socket = INVALID_SOCKET;
	Address _serverAddress = Address("127.0.0.1", htons(1234));
	ClientState _currentState = ClientState::Disconnected;
	const float _serverMaxInactivityTimeout;
	float _serverInactivityTimeLeft;
	uint64_t _saltNumber;
	uint64_t _dataPrefix;
	unsigned int _clientIndex;

	std::vector<Message*> _pendingMessages;
	std::queue<Message*> _sentMessages;
};

