#pragma once
#include "numeric_types.h"

#include "core/address.h"
#include "transmission_channels/unreliable_unordered_transmission_channel.h"
#include "metrics/metrics_handler.h"
#include "connection/connection_failed_reason_type.h"

namespace NetLib
{
	class MessageFactory;
	class Socket;
	class NetworkPacket;

	namespace Connection
	{
		enum class PendingConnectionState : uint8
		{
			Initializing = 0,
			ConnectionChallenge = 1,
			Completed = 2,
			Failed = 3
		};

		class PendingConnection
		{
			public:
				PendingConnection();
				PendingConnection( MessageFactory* message_factory );

				bool StartUp( const Address& address, bool started_locally );
				bool ShutDown();

				void ProcessPacket( NetworkPacket& packet );

				PendingConnectionState GetCurrentState() const { return _currentState; };
				void SetCurrentState( PendingConnectionState new_state ) { _currentState = new_state; };

				const Message* GetPendingReadyToProcessMessage();
				bool AddMessage( std::unique_ptr< Message > message );
				bool AddReceivedMessage( std::unique_ptr< Message > message );

				void SendData( Socket& socket );

				void UpdateConnectionElapsedTime( float32 elapsed_time )
				{
					_currentConnectionElapsedTimeSeconds += elapsed_time;
				};

				float32 GetCurrentConnectionElapsedTime() const { return _currentConnectionElapsedTimeSeconds; };

				uint64 GetClientSalt() const { return _clientSalt; };
				uint64 GetServerSalt() const { return _serverSalt; };
				uint64 GetDataPrefix() const { return _dataPrefix; };
				void GenerateDataPrefix() { _dataPrefix = _clientSalt ^ _serverSalt; };
				void SetClientSalt( uint64 client_salt )
				{
					_clientSalt = client_salt;
					_hasClientSaltAssigned = true;
				};
				void SetServerSalt( uint64 server_salt )
				{
					_serverSalt = server_salt;
					_hasServerSaltAssigned = true;
				};

				uint16 GetId() const { return _id; };
				void SetId( uint16 id ) { _id = id; };
				uint16 GetClientSideId() const { return _clientSideId; };
				void SetClientSideId( uint16 client_side_id ) { _clientSideId = client_side_id; };
				const Address& GetAddress() const { return _address; };
				bool WasStartedLocally() const { return _startedLocally; };
				bool HasClientSaltAssigned() const { return _hasClientSaltAssigned; };
				bool HasServerSaltAssigned() const { return _hasServerSaltAssigned; };
				ConnectionFailedReasonType GetConnectionDeniedReason() const { return _connectionDeniedReason; }
				void SetConnectionDeniedReason( ConnectionFailedReasonType reason )
				{
					_connectionDeniedReason = reason;
				};

			private:
				bool _isStartedUp;

				Address _address;
				PendingConnectionState _currentState;
				UnreliableUnorderedTransmissionChannel _transmissionChannel;
				Metrics::MetricsHandler _metricsHandler;

				uint64 _clientSalt;
				bool _hasClientSaltAssigned;
				uint64 _serverSalt;
				bool _hasServerSaltAssigned;
				uint64 _dataPrefix;
				uint16 _id;
				uint16 _clientSideId;
				bool _startedLocally;
				float32 _currentConnectionElapsedTimeSeconds;
				ConnectionFailedReasonType _connectionDeniedReason;
		};
	} // namespace Connection
} // namespace NetLib
