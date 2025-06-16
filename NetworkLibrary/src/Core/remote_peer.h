#pragma once
#include <map>
#include <memory>

#include "numeric_types.h"
#include "logger.h"

#include "communication/network_packet.h"

#include "core/address.h"
#include "core/ping_pong_messages_sender.h"

#include "metrics/metrics_handler.h"

#include "transmission_channels/transmission_channel.h"

namespace NetLib
{
	class Message;
	struct MessageHeader;

	enum RemotePeerState : uint8
	{
		Disconnected = 0,
		Connected = 1,
		Connecting = 2
	};

	class RemotePeer
	{
		private:
			Address _address;
			uint16 _id = 0;
			RemotePeerState _currentState;

			float32 _maxInactivityTime;
			float32 _inactivityTimeLeft;
			uint64 _clientSalt;
			uint64 _serverSalt;

			uint16 _nextPacketSequenceNumber;

			std::vector< TransmissionChannel* > _transmissionChannels;

			bool _metricsEnabled;
			Metrics::MetricsHandler _metricsHandler;

			PingPongMessagesSender _pingPongMessagesSender;

			void InitTransmissionChannels();
			TransmissionChannel* GetTransmissionChannelFromType( TransmissionChannelType channelType );
			const TransmissionChannel* GetTransmissionChannelFromType( TransmissionChannelType channelType ) const;
			TransmissionChannelType GetTransmissionChannelTypeFromHeader( const MessageHeader& messageHeader ) const;

		public:
			RemotePeer();
			RemotePeer( const Address& address, uint16 id, float32 maxInactivityTime, uint64 clientSalt,
			            uint64 serverSalt );
			RemotePeer( const RemotePeer& ) = delete;
			RemotePeer( RemotePeer&& other ) = default; // This must be here since Peer.h has a std::vector<RemotePeer>
			                                            // and vector<T> requires T to be MoveAssignable

			RemotePeer& operator=( const RemotePeer& ) = delete;
			~RemotePeer();

			uint16 GetLastMessageSequenceNumberAcked( TransmissionChannelType channelType ) const;

			void ActivateNetworkStatistics();
			void DeactivateNetworkStatistics();

			/// <summary>
			/// Initializes all the internal systems. You must call this method before performing any other operation.
			/// It is also automatically called in parameterized ctor
			/// </summary>
			/// <param name="addressInfo">Information about the address of this remote client</param>
			/// <param name="id">Client's unique ID</param>
			/// <param name="maxInactivityTime">Maximum number of seconds without receiving information from this client
			/// to consider it "Inactive"</param> <param name="dataPrefix">Client's data encryption prefix generated
			/// during the connection pipeline</param>
			void Connect( const Address& address, uint16 id, float32 maxInactivityTime, uint64 clientSalt,
			              uint64 serverSalt );

			void SetConnected() { _currentState = RemotePeerState::Connected; }

			void Tick( float32 elapsedTime );

			const Address& GetAddress() const { return _address; }
			uint16 GetClientIndex() const { return _id; }
			uint64 GetDataPrefix() const
			{
				return _clientSalt ^ _serverSalt;
			} // XOR operation to create the data prefix for all packects from now on
			uint64 GetClientSalt() const { return _clientSalt; }
			uint64 GetServerSalt() const { return _serverSalt; }
			RemotePeerState GeturrentState() const { return _currentState; }

			void SetServerSalt( uint64 newValue ) { _serverSalt = newValue; }

			bool IsAddressEqual( const Address& other ) const { return other == _address; }
			bool IsInactive() const { return _inactivityTimeLeft == 0.f; }
			bool AddMessage( std::unique_ptr< Message > message );
			bool ArePendingMessages( TransmissionChannelType channelType ) const;
			std::unique_ptr< Message > GetPendingMessage( TransmissionChannelType channelType );
			uint32 GetSizeOfNextUnsentMessage( TransmissionChannelType channelType ) const;
			void AddSentMessage( std::unique_ptr< Message > message, TransmissionChannelType channelType );
			void FreeSentMessages();
			void FreeProcessedMessages();
			void SeUnsentACKsToFalse( TransmissionChannelType channelType );
			bool AreUnsentACKs( TransmissionChannelType channelType ) const;
			uint32 GenerateACKs( TransmissionChannelType channelType ) const;
			void ProcessPacket( NetworkPacket& packet );
			void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber, TransmissionChannelType channelType );
			bool AddReceivedMessage( std::unique_ptr< Message > message );

			bool ArePendingReadyToProcessMessages() const;
			const Message* GetPendingReadyToProcessMessage();

			uint32 GetRTTMilliseconds() const;

			std::vector< TransmissionChannelType > GetAvailableTransmissionChannelTypes() const;
			uint32 GetNumberOfTransmissionChannels() const;

			uint32 GetMetric( const std::string& metric_name, const std::string& value_type ) const;

			/// <summary>
			/// Disconnect and reset the remote client
			/// </summary>
			void Disconnect();
	};
} // namespace NetLib
