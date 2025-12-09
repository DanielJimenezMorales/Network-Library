#pragma once
#include "numeric_types.h"

#include "core/address.h"
#include "transmission_channels/unreliable_unordered_transmission_channel.h"
#include "metrics/metrics_handler.h"

namespace NetLib
{
	class MessageFactory;

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

			bool StartUp( const Address& address );
			bool ShutDown();

			PendingConnectionState GetCurrentState() const { return _currentState; };
			void SetCurrentState( PendingConnectionState new_state ) { _currentState = new_state; };

			const Message* GetPendingReadyToProcessMessage();
			bool AddMessage( std::unique_ptr< Message > message );
			bool AddReceivedMessage( std::unique_ptr< Message > message );

			uint64 GetClientSalt() const { return _clientSalt; };
			uint64 GetServerSalt() const { return _serverSalt; };
			uint64 GetDataPrefix() const { return _dataPrefix; };
			void GenerateDataPrefix() { _dataPrefix = _clientSalt ^ _serverSalt; };
			void SetClientSalt( uint64 client_salt ) { _clientSalt = client_salt; };
			void SetServerSalt( uint64 server_salt ) { _serverSalt = server_salt; };
			uint16 GetId() const { return _id; };
			void SetId( uint16 id ) { _id = id; };
			const Address& GetAddress() const { return _address; };

		private:
			Address _address;
			PendingConnectionState _currentState;
			UnreliableUnorderedTransmissionChannel _transmissionChannel;
			Metrics::MetricsHandler _metricsHandler;

			uint64 _clientSalt;
			uint64 _serverSalt;
			uint64 _dataPrefix;
			uint16 _id;
	};
} // namespace NetLib
