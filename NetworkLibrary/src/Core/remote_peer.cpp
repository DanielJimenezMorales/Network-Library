#include "remote_peer.h"

#include <cassert>
#include <memory>

#include "communication/message.h"
#include "communication/message_factory.h"

#include "transmission_channels/unreliable_ordered_transmission_channel.h"
#include "transmission_channels/unreliable_unordered_transmission_channel.h"
#include "transmission_channels/reliable_ordered_channel.h"

namespace NetLib
{
	void RemotePeer::InitTransmissionChannels()
	{
		TransmissionChannel* unreliableOrdered = new UnreliableOrderedTransmissionChannel();
		TransmissionChannel* unreliableUnordered = new UnreliableUnorderedTransmissionChannel();
		TransmissionChannel* reliableOrdered = new ReliableOrderedChannel();

		_transmissionChannels.push_back( unreliableOrdered );
		_transmissionChannels.push_back( unreliableUnordered );
		_transmissionChannels.push_back( reliableOrdered );
	}

	TransmissionChannel* RemotePeer::GetTransmissionChannelFromType( TransmissionChannelType channelType )
	{
		TransmissionChannel* transmissionChannel = nullptr;

		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			if ( _transmissionChannels[ i ]->GetType() == channelType )
			{
				transmissionChannel = _transmissionChannels[ i ];
				break;
			}
		}

		return transmissionChannel;
	}

	const TransmissionChannel* RemotePeer::GetTransmissionChannelFromType( TransmissionChannelType channelType ) const
	{
		const TransmissionChannel* transmissionChannel = nullptr;

		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			if ( _transmissionChannels[ i ]->GetType() == channelType )
			{
				transmissionChannel = _transmissionChannels[ i ];
				break;
			}
		}

		return transmissionChannel;
	}

	RemotePeer::RemotePeer()
	    : _address( Address::GetInvalid() )
	    , _clientSalt( 0 )
	    , _serverSalt( 0 )
	    , _maxInactivityTime( 0 )
	    , _inactivityTimeLeft( 0 )
	    , _nextPacketSequenceNumber( 0 )
	    , _currentState( RemotePeerState::Disconnected )
	    , _transmissionChannels()
	    , _metricsEnabled( false )
	{
		InitTransmissionChannels();
	}

	RemotePeer::RemotePeer( const Address& address, uint16 id, float32 maxInactivityTime, uint64 clientSalt,
	                        uint64 serverSalt )
	    : _address( Address::GetInvalid() )
	    , _nextPacketSequenceNumber( 0 )
	    , _currentState( RemotePeerState::Disconnected )
	    , _metricsEnabled( false )
	{
		InitTransmissionChannels();
		Connect( address, id, maxInactivityTime, clientSalt, serverSalt );
	}

	RemotePeer::~RemotePeer()
	{
		Disconnect();

		// Free transmission channel memory
		for ( uint32 i = 0; i < _transmissionChannels.size(); ++i )
		{
			delete _transmissionChannels[ i ];
			_transmissionChannels[ i ] = nullptr;
		}

		_transmissionChannels.clear();
	}

	uint16 RemotePeer::GetLastMessageSequenceNumberAcked( TransmissionChannelType channelType ) const
	{
		uint16 lastMessageSequenceNumberAcked = 0;

		const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			lastMessageSequenceNumberAcked = transmissionChannel->GetLastMessageSequenceNumberAcked();
		}

		return lastMessageSequenceNumberAcked;
	}

	void RemotePeer::ActivateNetworkStatistics()
	{
		_metricsEnabled = true;
		_metricsHandler.Configure( 1.f );
	}

	void RemotePeer::DeactivateNetworkStatistics()
	{
		_metricsEnabled = false;
	}

	void RemotePeer::Connect( const Address& address, uint16 id, float32 maxInactivityTime, uint64 clientSalt,
	                          uint64 serverSalt )
	{
		_address = address;
		_id = id;
		_maxInactivityTime = maxInactivityTime;
		_inactivityTimeLeft = _maxInactivityTime;
		_clientSalt = clientSalt;
		_serverSalt = serverSalt;
		_currentState = RemotePeerState::Connecting;
	}

	void RemotePeer::Tick( float32 elapsedTime )
	{
		_inactivityTimeLeft -= elapsedTime;

		if ( _inactivityTimeLeft < 0.f )
		{
			_inactivityTimeLeft = 0.f;
		}

		// Update transmission channels
		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			_transmissionChannels[ i ]->Update( elapsedTime );
		}

		if ( _metricsEnabled )
		{
			_metricsHandler.Update( elapsedTime );
		}
	}

	bool RemotePeer::AddMessage( std::unique_ptr< Message > message )
	{
		TransmissionChannelType channelType = GetTransmissionChannelTypeFromHeader( message->GetHeader() );

		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			transmissionChannel->AddMessageToSend( std::move( message ) );
			return true;
		}
		else
		{
			// TODO En este caso ver qué hacer con el mensaje que nos pasan por parámetro
			return false;
		}
	}

	TransmissionChannelType RemotePeer::GetTransmissionChannelTypeFromHeader( const MessageHeader& messageHeader ) const
	{
		TransmissionChannelType result;

		if ( messageHeader.isReliable && messageHeader.isOrdered )
		{
			result = TransmissionChannelType::ReliableOrdered;
		}
		else if ( !messageHeader.isReliable && messageHeader.isOrdered )
		{
			result = TransmissionChannelType::UnreliableOrdered;
		}
		else if ( !messageHeader.isReliable && !messageHeader.isOrdered )
		{
			result = TransmissionChannelType::UnreliableUnordered;
		}

		return result;
	}

	bool RemotePeer::ArePendingMessages( TransmissionChannelType channelType ) const
	{
		bool arePendingMessages = false;

		const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			arePendingMessages = transmissionChannel->ArePendingMessagesToSend();
		}

		return arePendingMessages;
	}

	std::unique_ptr< Message > RemotePeer::GetPendingMessage( TransmissionChannelType channelType )
	{
		std::unique_ptr< Message > message = nullptr;

		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			message = transmissionChannel->GetMessageToSend();
		}

		return std::move( message );
	}

	uint32 RemotePeer::GetSizeOfNextUnsentMessage( TransmissionChannelType channelType ) const
	{
		uint32 size = 0;

		const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			size = transmissionChannel->GetSizeOfNextUnsentMessage();
		}

		return size;
	}

	void RemotePeer::FreeSentMessages()
	{
		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			_transmissionChannels[ i ]->FreeSentMessages();
		}
	}

	void RemotePeer::AddSentMessage( std::unique_ptr< Message > message, TransmissionChannelType channelType )
	{
		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			transmissionChannel->AddSentMessage( std::move( message ) );
		}
	}

	void RemotePeer::FreeProcessedMessages()
	{
		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			_transmissionChannels[ i ]->FreeProcessedMessages();
		}
	}

	void RemotePeer::SeUnsentACKsToFalse( TransmissionChannelType channelType )
	{
		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			transmissionChannel->SeUnsentACKsToFalse();
		}
	}

	bool RemotePeer::AreUnsentACKs( TransmissionChannelType channelType ) const
	{
		bool areUnsentACKs = false;

		const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			areUnsentACKs = transmissionChannel->AreUnsentACKs();
		}

		return areUnsentACKs;
	}

	uint32 RemotePeer::GenerateACKs( TransmissionChannelType channelType ) const
	{
		uint32 acks = 0;

		const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			acks = transmissionChannel->GenerateACKs();
		}

		return acks;
	}

	void RemotePeer::ProcessPacket( NetworkPacket& packet )
	{
		const uint32 packet_size = packet.Size();

		// Process packet ACKs
		uint32 acks = packet.GetHeader().ackBits;
		uint16 lastAckedMessageSequenceNumber = packet.GetHeader().lastAckedSequenceNumber;
		TransmissionChannelType channelType = static_cast< TransmissionChannelType >( packet.GetHeader().channelType );
		ProcessACKs( acks, lastAckedMessageSequenceNumber, channelType );

		// Process packet messages one by one
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		while ( packet.GetNumberOfMessages() > 0 )
		{
			std::unique_ptr< Message > message = packet.GetMessages();
			AddReceivedMessage( std::move( message ) );
		}

		if ( _metricsEnabled )
		{
			_metricsHandler.AddValue( "UPLOAD_BANDWIDTH", packet_size );
		}
	}

	void RemotePeer::ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
	                              TransmissionChannelType channelType )
	{
		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			Metrics::MetricsHandler* metricsHandler = _metricsEnabled ? &_metricsHandler : nullptr;
			transmissionChannel->ProcessACKs( acks, lastAckedMessageSequenceNumber, metricsHandler );
		}
	}

	bool RemotePeer::AddReceivedMessage( std::unique_ptr< Message > message )
	{
		bool result = false;
		TransmissionChannelType channelType = GetTransmissionChannelTypeFromHeader( message->GetHeader() );
		const uint32 messageSize = message->Size();

		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType( channelType );
		if ( transmissionChannel != nullptr )
		{
			transmissionChannel->AddReceivedMessage( std::move( message ) );
			_inactivityTimeLeft = _maxInactivityTime;
		}
		else
		{
			// TODO En este caso ver qué hacer con el mensaje que nos pasan por parámetro
		}

		return result;
	}

	bool RemotePeer::ArePendingReadyToProcessMessages() const
	{
		bool areReadyToProcessMessages = false;

		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			if ( _transmissionChannels[ i ]->ArePendingReadyToProcessMessages() )
			{
				areReadyToProcessMessages = true;
				break;
			}
		}

		return areReadyToProcessMessages;
	}

	const Message* RemotePeer::GetPendingReadyToProcessMessage()
	{
		const Message* message = nullptr;

		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			if ( _transmissionChannels[ i ]->ArePendingReadyToProcessMessages() )
			{
				message = _transmissionChannels[ i ]->GetReadyToProcessMessage();
				break;
			}
		}

		return message;
	}

	std::vector< TransmissionChannelType > RemotePeer::GetAvailableTransmissionChannelTypes() const
	{
		std::vector< TransmissionChannelType > channelTypes;
		channelTypes.reserve( GetNumberOfTransmissionChannels() );

		std::vector< TransmissionChannel* >::const_iterator cit = _transmissionChannels.cbegin();
		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			channelTypes.push_back( ( *( cit + i ) )->GetType() );
		}

		return channelTypes;
	}

	uint32 RemotePeer::GetNumberOfTransmissionChannels() const
	{
		return _transmissionChannels.size();
	}

	uint32 RemotePeer::GetRTTMilliseconds() const
	{
		uint32 rtt = 0;
		uint32 numberOfTransmissionChannels = 0;

		for ( uint32 i = 0; i < numberOfTransmissionChannels; ++i )
		{
			uint32 transmissionChannelRTT = _transmissionChannels[ i ]->GetRTTMilliseconds();
			if ( transmissionChannelRTT > 0 )
			{
				rtt += transmissionChannelRTT;
				++numberOfTransmissionChannels;
			}
		}

		if ( numberOfTransmissionChannels > 0 )
		{
			rtt /= numberOfTransmissionChannels;
		}

		return rtt;
	}

	void RemotePeer::Disconnect()
	{
		// Reset transmission channels
		for ( uint32 i = 0; i < GetNumberOfTransmissionChannels(); ++i )
		{
			_transmissionChannels[ i ]->Reset();
		}

		// Reset address
		_address = Address::GetInvalid();

		_currentState = RemotePeerState::Disconnected;

		DeactivateNetworkStatistics();
	}
} // namespace NetLib
