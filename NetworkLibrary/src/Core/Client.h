#pragma once

//Define this before any include in order to be able to use std::numeric_limits<uint64_t>::min() and std::numeric_limits<uint64_t>::max() methods and not getting errors with the ones
//from Windows.h
#define NOMINMAX
#include "Peer.h"
#include "Address.h"

class Message;
class ConnectionChallengeMessage;
class ConnectionAcceptedMessage;
class ConnectionDeniedMessage;
class DisconnectionMessage;
class InGameResponseMessage;

enum ClientState
{
	Disconnected = 0,
	Connected = 1,
	SendingConnectionRequest = 2,
	SendingConnectionChallengeResponse = 3,
};

class Client : public Peer
{
public:
	Client(float serverMaxInactivityTimeout);
	~Client() override;
	
protected:
	bool StartConcrete() override;
	void ProcessMessage(const Message& message, const Address& address) override;
	void TickConcrete(float elapsedTime) override;
	void DisconnectRemotePeerConcrete(RemotePeer& remotePeer) override;
	bool StopConcrete() override;

private:
	void GenerateClientSaltNumber();
	void ProcessConnectionChallenge(const ConnectionChallengeMessage& message);
	void ProcessConnectionRequestAccepted(const ConnectionAcceptedMessage& message);
	void ProcessConnectionRequestDenied(const ConnectionDeniedMessage& message);
	void ProcessDisconnection(const DisconnectionMessage& message);
	void ProcessInGameResponse(const InGameResponseMessage& message);

	void CreateConnectionRequestMessage();
	void CreateConnectionChallengeResponse();
	void CreateInGameMessage();

	Address _serverAddress;
	ClientState _currentState = ClientState::Disconnected;
	const float _serverMaxInactivityTimeout;
	float _serverInactivityTimeLeft;
	uint64_t _saltNumber;
	uint64_t _dataPrefix;
	unsigned int _clientIndex;

	unsigned int inGameMessageID; //Only for RUDP testing purposes. Delete later!
};

