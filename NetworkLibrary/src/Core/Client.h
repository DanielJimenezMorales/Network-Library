#pragma once

// Define this before any include in order to be able to use std::numeric_limits<uint64>::min() and
// std::numeric_limits<uint64>::max() methods and not getting errors with the ones from Windows.h
#define NOMINMAX
#include <list>

#include "core/peer.h"
#include "core/address.h"

#include "replication/replication_messages_processor.h"

namespace NetLib
{
	class ConnectionChallengeMessage;
	class ConnectionAcceptedMessage;
	class ConnectionDeniedMessage;
	class DisconnectionMessage;
	class TimeResponseMessage;
	class ReplicationMessage;
	class IInputState;

	enum ClientState
	{
		CS_Disconnected = 0,
		CS_Connected = 1,
		CS_SendingConnectionRequest = 2,
		CS_SendingConnectionChallengeResponse = 3,
	};

	// TIME SYNC CONSTANTS
	// Number of RTT to calculate an average RTT for adjusting Server's clock delta time
	const uint32 TIME_REQUEST_RTT_BUFFER_SIZE = 10;
	// In order to get an accurate clock sync within the first ticks, the client will send a burst of time requests to
	// calculate a Server's clock delta time rapidly. Note: In this case, the '+ 1' is in case one of the burst messages
	// get lost
	const uint32 NUMBER_OF_INITIAL_TIME_REQUESTS_BURST = TIME_REQUEST_RTT_BUFFER_SIZE + 1;
	// This will discard the X biggest and smallest RTTs from the Adjusted RTT in order to get rid of possible outliers.
	// This value must be smaller than half TIME_REQUEST_RTT_BUFFER_SIZE
	const uint32 NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE = 1;
	// How often the client will send a time request message to adjust Server's clock delta time
	const float32 TIME_REQUESTS_FREQUENCY_SECONDS = 1.0f;

	class Client : public Peer
	{
		public:
			Client( float32 serverMaxInactivityTimeout );
			Client( const Client& ) = delete;

			Client& operator=( const Client& ) = delete;

			~Client() override;

			void SendInputs( const IInputState& inputState );
			uint32 GetLocalClientId() const;

		protected:
			bool StartConcrete() override;
			void ProcessMessageFromPeer( const Message& message, RemotePeer& remotePeer ) override;
			void ProcessMessageFromUnknownPeer( const Message& message, const Address& address ) override;
			void TickConcrete( float32 elapsedTime ) override;
			bool StopConcrete() override;

		private:
			uint64 GenerateClientSaltNumber();
			void ProcessConnectionChallenge( const ConnectionChallengeMessage& message, RemotePeer& remotePeer );
			void ProcessConnectionRequestAccepted( const ConnectionAcceptedMessage& message, RemotePeer& remotePeer );
			void ProcessConnectionRequestDenied( const ConnectionDeniedMessage& message );
			void ProcessDisconnection( const DisconnectionMessage& message, RemotePeer& remotePeer );
			void ProcessTimeResponse( const TimeResponseMessage& message );
			void ProcessReplicationAction( const ReplicationMessage& message );

			void CreateConnectionRequestMessage( RemotePeer& remotePeer );
			void CreateConnectionChallengeResponse( RemotePeer& remotePeer );
			void CreateTimeRequestMessage( RemotePeer& remotePeer );

			void UpdateTimeRequestsElapsedTime( float32 elapsedTime );

			void OnServerDisconnect();

			Address _serverAddress;
			ClientState _currentState;
			uint32 _clientIndex;

			uint32 inGameMessageID; // Only for RUDP testing purposes. Delete later!

			// Time requests related
			float32 _timeSinceLastTimeRequest;
			uint32 _numberOfInitialTimeRequestBurstLeft;
			std::list< uint32 > _timeRequestRTTs;

			ReplicationMessagesProcessor _replicationMessagesProcessor;
	};
} // namespace NetLib
