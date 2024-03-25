#pragma once

//Define this before any include in order to be able to use std::numeric_limits<uint64_t>::min() and std::numeric_limits<uint64_t>::max() methods and not getting errors with the ones
//from Windows.h
#define NOMINMAX
#include <list>

#include "Peer.h"
#include "Address.h"

namespace NetLib
{

	class Message;
	class ConnectionChallengeMessage;
	class ConnectionAcceptedMessage;
	class ConnectionDeniedMessage;
	class DisconnectionMessage;
	class TimeResponseMessage;
	class InGameResponseMessage;

	enum ClientState
	{
		Disconnected = 0,
		Connected = 1,
		SendingConnectionRequest = 2,
		SendingConnectionChallengeResponse = 3,
	};

	//TIME SYNC CONSTANTS
	//Number of RTT to calculate an average RTT for adjusting Server's clock delta time
	const unsigned int TIME_REQUEST_RTT_BUFFER_SIZE = 10;
	//In order to get an accurate clock sync within the first ticks, the client will send a burst of time requests to calculate a Server's clock delta time rapidly.
	//Note: In this case, the '+ 1' is in case one of the burst messages get lost
	const unsigned int NUMBER_OF_INITIAL_TIME_REQUESTS_BURST = TIME_REQUEST_RTT_BUFFER_SIZE + 1;
	//This will discard the X biggest and smallest RTTs from the Adjusted RTT in order to get rid of possible outliers. This value must be smaller than half TIME_REQUEST_RTT_BUFFER_SIZE
	const unsigned int NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE = 1;
	//How often the client will send a time request message to adjust Server's clock delta time
	const float TIME_REQUESTS_FREQUENCY_SECONDS = 1.0f;

	class Client : public Peer
	{
	public:
		Client(float serverMaxInactivityTimeout);
		Client(const Client&) = delete;

		Client& operator=(const Client&) = delete;

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
		void ProcessTimeResponse(const TimeResponseMessage& message);
		void ProcessInGameResponse(const InGameResponseMessage& message);

		void CreateConnectionRequestMessage();
		void CreateConnectionChallengeResponse();
		void CreateTimeRequestMessage();
		void CreateInGameMessage();

		void UpdateTimeRequestsElapsedTime(float elapsedTime);

		Address _serverAddress;
		ClientState _currentState = ClientState::Disconnected;
		const float _serverMaxInactivityTimeout;
		float _serverInactivityTimeLeft;
		uint64_t _saltNumber;
		uint64_t _dataPrefix;
		unsigned int _clientIndex;

		unsigned int inGameMessageID; //Only for RUDP testing purposes. Delete later!

		//Time requests related
		float _timeSinceLastTimeRequest;
		unsigned int _numberOfInitialTimeRequestBurstLeft;
		std::list<unsigned int> _timeRequestRTTs;
	};
}
