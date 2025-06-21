#include "reliable_ordered_channel.h"

#include <memory>

#include "communication/message.h"
#include "communication/message_factory.h"
#include "communication/network_packet.h"

#include "utils/bitwise_utils.h"

#include "core/time_clock.h"
#include "core/Buffer.h"
#include "core/socket.h"
#include "core/address.h"

#include "metrics/metric_names.h"
#include "metrics/metrics_handler.h"

#include "logger.h"
#include "AlgorithmUtils.h"

namespace NetLib
{
	ReliableOrderedChannel::ReliableOrderedChannel()
	    : TransmissionChannel( TransmissionChannelType::ReliableOrdered )
	    , _lastMessageSequenceNumberAcked( 0 )
	    , _nextOrderedMessageSequenceNumber( 1 )
	    , _reliableMessageEntriesBufferSize( 1024 )
	    , _areUnsentACKs( false )
	    , _rttMilliseconds( 0 )
	{
		_reliableMessageEntries.reserve( _reliableMessageEntriesBufferSize );
		for ( uint32 i = 0; i < _reliableMessageEntriesBufferSize; ++i )
		{
			_reliableMessageEntries.emplace_back();
		}
	}

	ReliableOrderedChannel::ReliableOrderedChannel( ReliableOrderedChannel&& other ) noexcept
	    : TransmissionChannel( std::move( other ) )
	    , _lastMessageSequenceNumberAcked( std::move( other._lastMessageSequenceNumberAcked ) )
	    , // unnecessary move, just in case I change that type
	    _nextOrderedMessageSequenceNumber( std::move( other._nextOrderedMessageSequenceNumber ) )
	    , // unnecessary move, just in case I change that type
	    _reliableMessageEntriesBufferSize( std::move( other._reliableMessageEntriesBufferSize ) )
	    , // unnecessary move, just in case I change that type
	    _areUnsentACKs( std::move( other._areUnsentACKs ) )
	    , // unnecessary move, just in case I change that type
	    _rttMilliseconds( std::move( other._rttMilliseconds ) )
	    , // unnecessary move, just in case I change that type
	    _unackedReliableMessages( std::move( other._unackedReliableMessages ) )
	    , _unackedReliableMessageTimeouts( std::move( other._unackedReliableMessageTimeouts ) )
	    , _reliableMessageEntries( std::move( other._reliableMessageEntries ) )
	    , _unackedMessagesSendTimes( std::move( other._unackedMessagesSendTimes ) )
	    , _messagesRTTToProcess( std::move( other._messagesRTTToProcess ) )
	    , _orderedMessagesWaitingForPrevious( std::move( other._orderedMessagesWaitingForPrevious ) )
	{
	}

	ReliableOrderedChannel& ReliableOrderedChannel::operator=( ReliableOrderedChannel&& other ) noexcept
	{
		// Release old messages
		ClearMessages();

		// Move data from other to this
		_lastMessageSequenceNumberAcked =
		    std::move( other._lastMessageSequenceNumberAcked ); // unnecessary move, just in case I change that type
		_nextOrderedMessageSequenceNumber =
		    std::move( other._nextOrderedMessageSequenceNumber ); // unnecessary move, just in case I change that type
		_reliableMessageEntriesBufferSize =
		    std::move( other._reliableMessageEntriesBufferSize ); // unnecessary move, just in case I change that type
		_areUnsentACKs = std::move( other._areUnsentACKs );       // unnecessary move, just in case I change that type
		_rttMilliseconds = std::move( other._rttMilliseconds );   // unnecessary move, just in case I change that type
		_unackedReliableMessages = std::move( other._unackedReliableMessages );
		_unackedReliableMessageTimeouts = std::move( other._unackedReliableMessageTimeouts );
		_reliableMessageEntries = std::move( other._reliableMessageEntries );
		_unackedMessagesSendTimes = std::move( other._unackedMessagesSendTimes );
		_messagesRTTToProcess = std::move( other._messagesRTTToProcess );
		_orderedMessagesWaitingForPrevious = std::move( other._orderedMessagesWaitingForPrevious );

		TransmissionChannel::operator=( std::move( other ) );
		return *this;
	}

	bool ReliableOrderedChannel::GenerateAndSerializePacket( Socket& socket, const Address& address,
	                                                         Metrics::MetricsHandler* metrics_handler )
	{
		bool result = false;

		if ( !ArePendingMessagesToSend() && !AreUnsentACKs() )
		{
			return result;
		}

		NetworkPacket packet;

		// TODO Check somewhere if there is a message larger than the maximum packet size. Log a warning saying that the
		// message will never get sent and delete it.
		// TODO Include data prefix in packet's header and check if the data prefix is correct when receiving a packet

		// Check if we should include a message to the packet
		bool arePendingMessages = ArePendingMessagesToSend();
		bool isThereCapacityLeft = packet.CanMessageFit( GetSizeOfNextUnsentMessage() );

		while ( arePendingMessages && isThereCapacityLeft )
		{
			// Configure and add message to packet
			std::unique_ptr< Message > message = GetMessageToSend( metrics_handler );

			if ( message->GetHeader().isReliable )
			{
				LOG_INFO( "Reliable message sequence number: %hu, Message type: %hhu",
				          message->GetHeader().messageSequenceNumber, message->GetHeader().type );
			}

			packet.AddMessage( std::move( message ) );

			// Check if we should include another message to the packet
			arePendingMessages = ArePendingMessagesToSend();
			isThereCapacityLeft = packet.CanMessageFit( GetSizeOfNextUnsentMessage() );
		}

		// Set packet header
		const uint32 acks = GenerateACKs();
		packet.SetHeaderACKs( acks );
		const uint16 lastAckedMessageSequenceNumber = GetLastMessageSequenceNumberAcked();
		packet.SetHeaderLastAcked( lastAckedMessageSequenceNumber );
		packet.SetHeaderChannelType( GetType() );

		// Serialize packet
		uint8* bufferData = new uint8[ packet.Size() ];
		Buffer buffer( bufferData, packet.Size() );
		packet.Write( buffer );

		// Send packet
		socket.SendTo( buffer.GetData(), buffer.GetSize(), address );

		// TODO See what happens when the socket couldn't send the packet
		if ( metrics_handler != nullptr )
		{
			metrics_handler->AddValue( Metrics::UPLOAD_BANDWIDTH_METRIC, packet.Size() );
		}

		SeUnsentACKsToFalse();

		// Send messages ownership back to remote peer
		while ( packet.GetNumberOfMessages() > 0 )
		{
			std::unique_ptr< Message > message = packet.GetMessages();
			AddUnackedReliableMessage( std::move( message ) );
		}

		delete[] bufferData;

		result = true;
		return result;
	}

	void ReliableOrderedChannel::AddMessageToSend( std::unique_ptr< Message > message )
	{
		_unsentMessages.push_back( std::move( message ) );
	}

	bool ReliableOrderedChannel::ArePendingMessagesToSend() const
	{
		return ( !_unsentMessages.empty() || AreUnackedMessagesToResend() );
	}

	std::unique_ptr< Message > ReliableOrderedChannel::GetMessageToSend( Metrics::MetricsHandler* metrics_handler )
	{
		std::unique_ptr< Message > message = nullptr;
		if ( !_unsentMessages.empty() )
		{
			message = std::move( _unsentMessages[ 0 ] );
			_unsentMessages.erase( _unsentMessages.begin() );

			const uint16 sequenceNumber = GetNextMessageSequenceNumber();
			IncreaseMessageSequenceNumber();

			message->SetHeaderPacketSequenceNumber( sequenceNumber );
			SetUnackedMessageSendTime( sequenceNumber );
		}
		else
		{
			message = GetUnackedMessageToResend();
			if ( message != nullptr && metrics_handler != nullptr )
			{
				metrics_handler->AddValue( Metrics::RETRANSMISSION_METRIC, 1 );
			}
		}

		// TODO Check that this is not called when message == nullptr. GetUnackedMessageToResend could return a nullptr
		// (Although it would be an error tbh)

		return std::move( message );
	}

	uint32 ReliableOrderedChannel::GetSizeOfNextUnsentMessage() const
	{
		if ( !ArePendingMessagesToSend() && !AreUnackedMessagesToResend() )
		{
			return 0;
		}

		if ( !_unsentMessages.empty() )
		{
			return _unsentMessages.front()->Size();
		}
		else
		{
			// Get next unacked message's size
			int32 index = GetNextUnackedMessageIndexToResend();

			std::list< std::unique_ptr< Message > >::const_iterator cit = _unackedReliableMessages.cbegin();
			std::advance( cit, index );

			return ( *cit )->Size();
		}
	}

	void ReliableOrderedChannel::AddReceivedMessage( std::unique_ptr< Message > message,
	                                                 Metrics::MetricsHandler* metrics_handler )
	{
		const uint16 messageSequenceNumber = message->GetHeader().messageSequenceNumber;
		if ( IsMessageDuplicated( messageSequenceNumber ) )
		{
			LOG_INFO( "The message with ID = %hu is duplicated. Ignoring it...", messageSequenceNumber );

			MessageFactory& messageFactory = MessageFactory::GetInstance();
			messageFactory.ReleaseMessage( std::move( message ) );
			if ( metrics_handler != nullptr )
			{
				metrics_handler->AddValue( Metrics::DUPLICATE_METRIC, 1 );
			}
			return;
		}
		else
		{
			LOG_INFO( "New message received" );
			AckReliableMessage( messageSequenceNumber );
			if ( messageSequenceNumber == _nextOrderedMessageSequenceNumber )
			{
				_readyToProcessMessages.push( std::move( message ) );
				++_nextOrderedMessageSequenceNumber;

				bool continueProcessing = true;
				uint32 index = 0;
				while ( continueProcessing )
				{
					if ( _orderedMessagesWaitingForPrevious.empty() )
					{
						continueProcessing = false;
					}
					else if ( DoesUnorderedMessagesContainsSequence( _nextOrderedMessageSequenceNumber, index ) )
					{
						std::list< std::unique_ptr< Message > >::iterator it =
						    _orderedMessagesWaitingForPrevious.begin();
						std::advance( it, index );

						std::unique_ptr< Message > readyToProcessMessage( std::move( *it ) );
						_readyToProcessMessages.push( std::move( readyToProcessMessage ) );
						_orderedMessagesWaitingForPrevious.erase( it );
						++_nextOrderedMessageSequenceNumber;
					}
					else
					{
						continueProcessing = false;
					}
				}
			}
			else
			{
				AddOrderedMessage( std::move( message ) );
				if ( metrics_handler != nullptr )
				{
					metrics_handler->AddValue( Metrics::OUT_OF_ORDER_METRIC, 1 );
				}
			}
		}
	}

	bool ReliableOrderedChannel::ArePendingReadyToProcessMessages() const
	{
		return !_readyToProcessMessages.empty();
	}

	const Message* ReliableOrderedChannel::GetReadyToProcessMessage()
	{
		if ( !ArePendingReadyToProcessMessages() )
		{
			return nullptr;
		}

		std::unique_ptr< Message > message( std::move( _readyToProcessMessages.front() ) );
		_readyToProcessMessages.pop();

		Message* messageToReturn = message.get();
		_processedMessages.push( std::move( message ) );

		return messageToReturn;
	}

	bool ReliableOrderedChannel::AreUnackedMessagesToResend() const
	{
		std::list< float32 >::const_iterator cit = _unackedReliableMessageTimeouts.cbegin();
		bool found = false;
		while ( cit != _unackedReliableMessageTimeouts.cend() && !found )
		{
			float32 timeout = *cit;
			if ( timeout <= 0 )
			{
				found = true;
			}

			++cit;
		}

		return found;
	}

	std::unique_ptr< Message > ReliableOrderedChannel::GetUnackedMessageToResend()
	{
		int32 index = GetNextUnackedMessageIndexToResend();
		if ( index == -1 )
		{
			return nullptr;
		}

		std::unique_ptr< Message > message = DeleteUnackedReliableMessageAtIndex( index );

		return std::move( message );
	}

	int32 ReliableOrderedChannel::GetNextUnackedMessageIndexToResend() const
	{
		int32 index = 0;
		std::list< float32 >::const_iterator cit = _unackedReliableMessageTimeouts.cbegin();
		bool found = false;
		while ( cit != _unackedReliableMessageTimeouts.cend() && !found )
		{
			float32 timeout = *cit;
			if ( timeout <= 0 )
			{
				found = true;
			}
			else
			{
				++index;
				++cit;
			}
		}

		if ( !found )
		{
			index = -1;
		}

		return index;
	}

	void ReliableOrderedChannel::AddUnackedReliableMessage( std::unique_ptr< Message > message )
	{
		_unackedReliableMessages.push_back( std::move( message ) );
		LOG_INFO( "Retransmission Timeout: %f", GetRetransmissionTimeout() );
		_unackedReliableMessageTimeouts.push_back( GetRetransmissionTimeout() );
	}

	void ReliableOrderedChannel::AckReliableMessage( uint16 messageSequenceNumber )
	{
		uint32 index = GetRollingBufferIndex( messageSequenceNumber );
		_reliableMessageEntries[ index ].sequenceNumber = messageSequenceNumber;
		_reliableMessageEntries[ index ].isAcked = true;

		_lastMessageSequenceNumberAcked = messageSequenceNumber;

		// Set this flag to true so in case this peer does not have any relaible messages, force it so send a reliable
		// packet just to notify of new acked messages from remote
		_areUnsentACKs = true;
	}

	bool ReliableOrderedChannel::DoesUnorderedMessagesContainsSequence( uint16 sequence, uint32& index ) const
	{
		const Message* message = nullptr;
		uint32 idx = 0;
		for ( std::list< std::unique_ptr< Message > >::const_iterator cit = _orderedMessagesWaitingForPrevious.cbegin();
		      cit != _orderedMessagesWaitingForPrevious.cend(); ++cit )
		{
			message = ( *cit ).get();
			if ( message->GetHeader().messageSequenceNumber == sequence )
			{
				index = idx;
				return true;
			}

			++idx;
		}

		return false;
	}

	bool ReliableOrderedChannel::AddOrderedMessage( std::unique_ptr< Message > message )
	{
		_orderedMessagesWaitingForPrevious.push_back( std::move( message ) );
		return true;

		// TODO Ver para qué es esto de abajo y por qué este return de aquí arriba
		std::list< std::unique_ptr< Message > >::iterator it = _orderedMessagesWaitingForPrevious.begin();
		if ( _orderedMessagesWaitingForPrevious.empty() )
		{
			_orderedMessagesWaitingForPrevious.insert( it, std::move( message ) );
			return true;
		}

		bool found = false;
		while ( it != _orderedMessagesWaitingForPrevious.end() && !found )
		{
			uint16 sequenceNumber = ( *it )->GetHeader().messageSequenceNumber;
			if ( sequenceNumber > message->GetHeader().messageSequenceNumber )
			{
				found = true;
			}
			else
			{
				++it;
			}
		}

		_orderedMessagesWaitingForPrevious.insert( it, std::move( message ) );

		return found;
	}

	bool ReliableOrderedChannel::TryRemoveUnackedReliableMessageFromSequence( uint16 sequence,
	                                                                          Metrics::MetricsHandler* metrics_handler )
	{
		bool result = false;

		int32 index = GetPendingUnackedReliableMessageIndexFromSequence( sequence );
		if ( index != -1 )
		{
			// Calculate RTT of acked message
			const TimeClock& timeClock = TimeClock::GetInstance();
			const uint64 currentElapsedTime = timeClock.GetLocalTimeMilliseconds();
			const uint32 messageRTT = currentElapsedTime - _unackedMessagesSendTimes[ sequence ];
			AddMessageRTTValueToProcess( messageRTT );

			if ( metrics_handler != nullptr )
			{
				const uint32 latency = messageRTT / 2;
				metrics_handler->AddValue( Metrics::LATENCY_METRIC, latency );
				metrics_handler->AddValue( Metrics::JITTER_METRIC, latency );
			}

			std::unique_ptr< Message > message = DeleteUnackedReliableMessageAtIndex( index );

			std::unordered_map< uint16, uint32 >::iterator it = _unackedMessagesSendTimes.find( sequence );
			_unackedMessagesSendTimes.erase( it );

			// Release acked message since we no longer need it
			MessageFactory& messageFactory = MessageFactory::GetInstance();
			messageFactory.ReleaseMessage( std::move( message ) );
			result = true;
		}

		return result;
	}

	int32 ReliableOrderedChannel::GetPendingUnackedReliableMessageIndexFromSequence( uint16 sequence ) const
	{
		int32 resultIndex = -1;
		uint32 currentIndex = 0;
		for ( std::list< std::unique_ptr< Message > >::const_iterator it = _unackedReliableMessages.cbegin();
		      it != _unackedReliableMessages.cend(); ++it )
		{
			if ( ( *it )->GetHeader().messageSequenceNumber == sequence )
			{
				resultIndex = currentIndex;
				break;
			}

			++currentIndex;
		}

		return resultIndex;
	}

	std::unique_ptr< Message > ReliableOrderedChannel::DeleteUnackedReliableMessageAtIndex( uint32 index )
	{
		assert( index < _unackedReliableMessages.size() );

		std::list< std::unique_ptr< Message > >::iterator it = _unackedReliableMessages.begin();
		std::advance( it, index );
		std::unique_ptr< Message > message( std::move( *it ) );

		_unackedReliableMessages.erase( it );

		std::list< float32 >::iterator it2 = _unackedReliableMessageTimeouts.begin();
		std::advance( it2, index );
		_unackedReliableMessageTimeouts.erase( it2 );

		return std::move( message );
	}

	const ReliableMessageEntry& ReliableOrderedChannel::GetReliableMessageEntry( uint16 sequenceNumber ) const
	{
		uint32 index = GetRollingBufferIndex( sequenceNumber );
		return _reliableMessageEntries[ index ];
	}

	void ReliableOrderedChannel::AddMessageRTTValueToProcess( uint32 messageRTT )
	{
		_messagesRTTToProcess.push( messageRTT );
	}

	void ReliableOrderedChannel::UpdateRTT()
	{
		while ( !_messagesRTTToProcess.empty() )
		{
			const uint32 messageRTTValue = _messagesRTTToProcess.front();
			_messagesRTTToProcess.pop();

			if ( _rttMilliseconds == 0 )
			{
				_rttMilliseconds = messageRTTValue;
			}
			else
			{
				_rttMilliseconds =
				    Common::AlgorithmUtils::ExponentialMovingAverage( _rttMilliseconds, messageRTTValue, 10 );
			}
		}

		LOG_INFO( "RTT: %u", _rttMilliseconds );
	}

	float32 ReliableOrderedChannel::GetRetransmissionTimeout() const
	{
		if ( _rttMilliseconds == 0 )
		{
			return _initialTimeout;
		}

		return ( float32 ) _rttMilliseconds / 1000 * 2;
	}

	void ReliableOrderedChannel::SetUnackedMessageSendTime( uint16 sequence )
	{
		const TimeClock& timeClock = TimeClock::GetInstance();
		_unackedMessagesSendTimes[ sequence ] = timeClock.GetLocalTimeMilliseconds();
	}

	void ReliableOrderedChannel::ClearMessages()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::list< std::unique_ptr< Message > >::iterator it = _unackedReliableMessages.begin();
		while ( it != _unackedReliableMessages.end() )
		{
			std::unique_ptr< Message > message( std::move( *it ) );
			messageFactory.ReleaseMessage( std::move( message ) );

			++it;
		}

		_unackedReliableMessages.clear();

		_unackedReliableMessageTimeouts.clear();

		_unackedMessagesSendTimes.clear();

		it = _orderedMessagesWaitingForPrevious.begin();
		while ( it != _orderedMessagesWaitingForPrevious.end() )
		{
			std::unique_ptr< Message > message( std::move( *it ) );
			messageFactory.ReleaseMessage( std::move( message ) );

			++it;
		}

		_orderedMessagesWaitingForPrevious.clear();
	}

	void ReliableOrderedChannel::SeUnsentACKsToFalse()
	{
		_areUnsentACKs = false;
	}

	bool ReliableOrderedChannel::AreUnsentACKs() const
	{
		return _areUnsentACKs;
	}

	uint32 ReliableOrderedChannel::GenerateACKs() const
	{
		uint32 acks = 0;
		uint16 firstSequenceNumber = _lastMessageSequenceNumberAcked - 1;
		for ( uint32 i = 0; i < 32; ++i )
		{
			uint16 currentSequenceNumber = firstSequenceNumber - i;
			const ReliableMessageEntry& reliableMessageEntry = GetReliableMessageEntry( currentSequenceNumber );
			if ( reliableMessageEntry.isAcked && currentSequenceNumber == reliableMessageEntry.sequenceNumber )
			{
				BitwiseUtils::SetBitAtIndex( acks, i );
			}
		}
		return acks;
	}

	void ReliableOrderedChannel::ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
	                                          Metrics::MetricsHandler* metrics_handler )
	{
		LOG_INFO( "Last acked from client = %hu", lastAckedMessageSequenceNumber );

		// Check if the last acked is in reliable messages lists
		TryRemoveUnackedReliableMessageFromSequence( lastAckedMessageSequenceNumber, metrics_handler );

		// Check for the rest of acked bits
		uint16 firstAckSequence = lastAckedMessageSequenceNumber - 1;
		for ( uint32 i = 0; i < 32; ++i )
		{
			if ( BitwiseUtils::GetBitAtIndex( acks, i ) )
			{
				TryRemoveUnackedReliableMessageFromSequence( firstAckSequence - i, metrics_handler );
			}
		}
	}

	bool ReliableOrderedChannel::IsMessageDuplicated( uint16 messageSequenceNumber ) const
	{
		bool result = false;

		uint32 index = GetRollingBufferIndex( messageSequenceNumber );
		if ( _reliableMessageEntries[ index ].sequenceNumber == messageSequenceNumber &&
		     _reliableMessageEntries[ index ].isAcked )
		{
			result = true;
		}

		return result;
	}

	void ReliableOrderedChannel::Update( float32 deltaTime )
	{
		// Update unacked message timeouts
		std::list< float32 >::iterator it = _unackedReliableMessageTimeouts.begin();
		while ( it != _unackedReliableMessageTimeouts.end() )
		{
			float32 timeout = *it;
			timeout -= deltaTime;

			if ( timeout < 0 )
			{
				timeout = 0;
			}

			*it = timeout;

			++it;
		}

		// Update RTT
		UpdateRTT();
	}

	uint16 ReliableOrderedChannel::GetLastMessageSequenceNumberAcked() const
	{
		return _lastMessageSequenceNumberAcked;
	}

	void ReliableOrderedChannel::Reset()
	{
		TransmissionChannel::Reset();
		ClearMessages();
		_lastMessageSequenceNumberAcked = 0;
		_nextOrderedMessageSequenceNumber = 1;
		_areUnsentACKs = false;
		_rttMilliseconds = 0;

		while ( !_messagesRTTToProcess.empty() )
		{
			_messagesRTTToProcess.pop();
		}

		for ( uint32 i = 0; i < _reliableMessageEntriesBufferSize; ++i )
		{
			_reliableMessageEntries[ i ].Reset();
		}
	}

	uint32 ReliableOrderedChannel::GetRTTMilliseconds() const
	{
		return _rttMilliseconds;
	}

	ReliableOrderedChannel::~ReliableOrderedChannel()
	{
		ClearMessages();
	}
} // namespace NetLib
