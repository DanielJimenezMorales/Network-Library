#pragma once

//Define this before any include in order to be able to use std::numeric_limits<uint64_t>::min() and std::numeric_limits<uint64_t>::max() methods and not getting errors with the ones
//from Windows.h
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Buffer.h"
#include "Address.h"

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
	void ProcessConnectionChallenge(Buffer& buffer);
	void ProcessConnectionRequestAccepted(Buffer& buffer);
	void ProcessConnectionRequestDenied();
	void ProcessDisconnection(Buffer& buffer);

	void SendConnectionRequestPacket();
	void SendPacketToServer(const Buffer& buffer) const;

	SOCKET _socket = INVALID_SOCKET;
	Address* _serverAddress;
	ClientState _currentState = ClientState::Disconnected;
	const float _serverMaxInactivityTimeout;
	float _serverInactivityTimeLeft;
	uint64_t _saltNumber;
	uint64_t _dataPrefix;
	unsigned int _clientIndex;
};

