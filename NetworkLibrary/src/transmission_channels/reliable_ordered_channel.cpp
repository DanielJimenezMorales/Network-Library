#include "reliable_ordered_channel.h"

#include <memory>
#include <cassert>

#include "communication/message.h"
#include "communication/message_factory.h"
#include "communication/network_packet.h"

#include "utils/bitwise_utils.h"

#include "core/time_clock.h"
#include "core/Buffer.h"
#include "core/socket.h"
#include "core/address.h"

#include "metrics/metrics_handler.h"
#include "metrics/metric_types.h"

#include "logger.h"
#include "AlgorithmUtils.h"

namespace NetLib
{
	ReliableOrderedChannel::ReliableOrderedChannel( MessageFactory* message_factory )
	    : TransmissionChannel( TransmissionChannelType::ReliableOrdered, message_factory )
	    , _lastAckedMessageSequenceNumber( 0 )
	    , _nextOrderedMessageSequenceNumber( 1 )
	    , _reliableMessageEntriesBufferSize( ACK_BITS_SIZE )
	    , _areUnsentACKs( false )
	    , _rttMilliseconds( 0 )
	{
		_remotePeerReliableMessageEntries.reserve( _reliableMessageEntriesBufferSize );
		for ( uint32 i = 0; i < _reliableMessageEntriesBufferSize; ++i )
		{
			_remotePeerReliableMessageEntries.emplace_back();
		}
	}

	ReliableOrderedChannel::ReliableOrderedChannel( ReliableOrderedChannel&& other ) noexcept
	    : TransmissionChannel( std::move( other ) )
	    , _lastAckedMessageSequenceNumber( std::move( other._lastAckedMessageSequenceNumber ) )
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
	    , _remotePeerReliableMessageEntries( std::move( other._remotePeerReliableMessageEntries ) )
	    , _unackedMessagesSendTimes( std::move( other._unackedMessagesSendTimes ) )
	    , _unorderedMessagesWaitingForPrevious( std::move( other._unorderedMessagesWaitingForPrevious ) )
	{
	}

	ReliableOrderedChannel& ReliableOrderedChannel::operator=( ReliableOrderedChannel&& other ) noexcept
	{
		// Release old messages
		ClearMessages();

		// Move data from other to this
		_lastAckedMessageSequenceNumber =
		    std::move( other._lastAckedMessageSequenceNumber ); // unnecessary move, just in case I change that type
		_nextOrderedMessageSequenceNumber =
		    std::move( other._nextOrderedMessageSequenceNumber ); // unnecessary move, just in case I change that type
		_reliableMessageEntriesBufferSize =
		    std::move( other._reliableMessageEntriesBufferSize ); // unnecessary move, just in case I change that type
		_areUnsentACKs = std::move( other._areUnsentACKs );       // unnecessary move, just in case I change that type
		_rttMilliseconds = std::move( other._rttMilliseconds );   // unnecessary move, just in case I change that type
		_unackedReliableMessages = std::move( other._unackedReliableMessages );
		_unackedReliableMessageTimeouts = std::move( other._unackedReliableMessageTimeouts );
		_remotePeerReliableMessageEntries = std::move( other._remotePeerReliableMessageEntries );
		_unackedMessagesSendTimes = std::move( other._unackedMessagesSendTimes );
		_unorderedMessagesWaitingForPrevious = std::move( other._unorderedMessagesWaitingForPrevious );

		TransmissionChannel::operator=( std::move( other ) );
		return *this;
	}

	bool ReliableOrderedChannel::CreateAndSendPacket( Socket& socket, const Address& address,
	                                                  Metrics::MetricsHandler& metrics_handler )
	{
		bool result = false;

		if ( !ArePendingMessagesToSend() && !_areUnsentACKs )
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
		packet.SetHeaderLastAcked( _lastAckedMessageSequenceNumber );
		packet.SetHeaderChannelType( GetType() );

		// Serialize packet
		uint8* bufferData = new uint8[ packet.Size() ];
		Buffer buffer( bufferData, packet.Size() );
		packet.Write( buffer );

		// Send packet
		socket.SendTo( buffer.GetData(), buffer.GetSize(), address );

		// TODO See what happens when the socket couldn't send the packet
		if ( metrics_handler.HasMetric( Metrics::MetricType::UPLOAD_BANDWIDTH ) )
		{
			metrics_handler.AddValue( Metrics::MetricType::UPLOAD_BANDWIDTH, packet.Size() );
		}

		_areUnsentACKs = false;

		// Send messages ownership back to remote peer
		while ( packet.GetNumberOfMessages() > 0 )
		{
			std::unique_ptr< Message > message = packet.TryGetNextMessage();
			AddUnackedMessage( std::move( message ) );

			if ( metrics_handler.HasMetric( Metrics::MetricType::PACKET_LOSS ) )
			{
				metrics_handler.AddValue( Metrics::MetricType::PACKET_LOSS, 1, "SENT" );
			}
		}

		delete[] bufferData;

		result = true;
		return result;
	}

	bool ReliableOrderedChannel::AddMessageToSend( std::unique_ptr< Message > message )
	{
		assert( message != nullptr );

		if ( !IsMessageSuitable( message->GetHeader() ) )
		{
			return false;
		}

		_unsentMessages.push_back( std::move( message ) );
		return true;
	}

	bool ReliableOrderedChannel::ArePendingMessagesToSend() const
	{
		return ( !_unsentMessages.empty() || AreUnackedMessagesToResend() );
	}

	std::unique_ptr< Message > ReliableOrderedChannel::GetMessageToSend( Metrics::MetricsHandler& metrics_handler )
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
			message = TryGetUnackedMessageToResend();
			if ( message != nullptr && metrics_handler.HasMetric( Metrics::MetricType::RETRANSMISSIONS ) )
			{
				metrics_handler.AddValue( Metrics::MetricType::RETRANSMISSIONS, 1 );
			}
		}

		// TODO Check that this is not called when message == nullptr. GetUnackedMessageToResend could return a nullptr
		// (Although it would be an error tbh)

		return std::move( message );
	}

	uint32 ReliableOrderedChannel::GetSizeOfNextUnsentMessage() const
	{
		if ( !ArePendingMessagesToSend() )
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
			int32 index = TryGetNextUnackedMessageIndexToResend();

			std::list< std::unique_ptr< Message > >::const_iterator cit = _unackedReliableMessages.cbegin();
			std::advance( cit, index );

			return ( *cit )->Size();
		}
	}

	bool ReliableOrderedChannel::IsMessageSuitable( const MessageHeader& header ) const
	{
		bool result = true;
		if ( !header.isReliable || !header.isOrdered )
		{
			LOG_WARNING( "Trying to add a message to a reliable ordered channel that is not suitable for it. "
			             "Message type: %hhu, isReliable: %u, isOrdered: %u",
			             header.type, header.isReliable, header.isOrdered );
			result = false;
		}

		return result;
	}

	bool ReliableOrderedChannel::AddReceivedMessage( std::unique_ptr< Message > message,
	                                                 Metrics::MetricsHandler& metrics_handler )
	{
		assert( message != nullptr );

		if ( !IsMessageSuitable( message->GetHeader() ) )
		{
			return false;
		}

		const uint16 messageSequenceNumber = message->GetHeader().messageSequenceNumber;
		if ( IsMessageDuplicated( messageSequenceNumber ) )
		{
			LOG_INFO( "The message with ID = %hu is duplicated. Ignoring it...", messageSequenceNumber );

			// Submit duplicate message metric
			if ( metrics_handler.HasMetric( Metrics::MetricType::DUPLICATE_MESSAGES ) )
			{
				metrics_handler.AddValue( Metrics::MetricType::DUPLICATE_MESSAGES, 1 );
			}

			// Release duplicate message
			_messageFactory->ReleaseMessage( std::move( message ) );
		}
		else
		{
			LOG_INFO( "New message received" );
			AckReliableMessage( messageSequenceNumber );
			if ( messageSequenceNumber == _nextOrderedMessageSequenceNumber )
			{
				ProcessOrderedMessage( std::move( message ) );
			}
			else
			{
				ProcessUnorderedMessage( std::move( message ), metrics_handler );
			}
		}

		return true;
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
			const float32 timeout = *cit;
			if ( timeout <= 0 )
			{
				found = true;
			}

			++cit;
		}

		return found;
	}

	std::unique_ptr< Message > ReliableOrderedChannel::TryGetUnackedMessageToResend()
	{
		const int32 index = TryGetNextUnackedMessageIndexToResend();
		if ( index == -1 )
		{
			return nullptr;
		}

		std::unique_ptr< Message > message = RemoveUnackedMessageFromBufferAtIndex( index );

		return std::move( message );
	}

	int32 ReliableOrderedChannel::TryGetNextUnackedMessageIndexToResend() const
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

	void ReliableOrderedChannel::AddUnackedMessage( std::unique_ptr< Message > message )
	{
		_unackedReliableMessages.push_back( std::move( message ) );
		const float32 retransmissionTimeout = GetRetransmissionTimeout();
		LOG_INFO( "Retransmission Timeout: %f", retransmissionTimeout );
		_unackedReliableMessageTimeouts.push_back( retransmissionTimeout );
	}

	bool ReliableOrderedChannel::DoesUnorderedMessagesBufferContainsSequenceNumber( uint16 sequence_number,
	                                                                                uint32& out_index ) const
	{
		const Message* message = nullptr;
		uint32 idx = 0;
		for ( std::list< std::unique_ptr< Message > >::const_iterator cit =
		          _unorderedMessagesWaitingForPrevious.cbegin();
		      cit != _unorderedMessagesWaitingForPrevious.cend(); ++cit )
		{
			message = ( *cit ).get();
			if ( message->GetHeader().messageSequenceNumber == sequence_number )
			{
				out_index = idx;
				return true;
			}

			++idx;
		}

		return false;
	}

	void ReliableOrderedChannel::ProcessOrderedMessage( std::unique_ptr< Message > message )
	{
		// Add message to the ready to be processed buffer
		_readyToProcessMessages.push( std::move( message ) );

		// Increment the next ordered message sequence number expected
		++_nextOrderedMessageSequenceNumber;

		// Check if with this new message received we can process other newer (out of order) messages received in the
		// previous states.
		bool continueProcessing = true;
		while ( !_unorderedMessagesWaitingForPrevious.empty() || continueProcessing )
		{
			uint32 index = 0;
			if ( DoesUnorderedMessagesBufferContainsSequenceNumber( _nextOrderedMessageSequenceNumber, index ) )
			{
				std::list< std::unique_ptr< Message > >::iterator it = _unorderedMessagesWaitingForPrevious.begin();
				std::advance( it, index );

				std::unique_ptr< Message > readyToProcessMessage( std::move( *it ) );
				_readyToProcessMessages.push( std::move( readyToProcessMessage ) );
				_unorderedMessagesWaitingForPrevious.erase( it );
				++_nextOrderedMessageSequenceNumber;
			}
			else
			{
				continueProcessing = false;
			}
		}
	}

	void ReliableOrderedChannel::ProcessUnorderedMessage( std::unique_ptr< Message > message,
	                                                      Metrics::MetricsHandler& metrics_handler )
	{
		AddUnorderedMessage( std::move( message ) );
		if ( metrics_handler.HasMetric( Metrics::MetricType::OUT_OF_ORDER_MESSAGES ) )
		{
			metrics_handler.AddValue( Metrics::MetricType::OUT_OF_ORDER_MESSAGES, 1 );
		}
	}

	void ReliableOrderedChannel::AddUnorderedMessage( std::unique_ptr< Message > message )
	{
		_unorderedMessagesWaitingForPrevious.push_back( std::move( message ) );
	}

	bool ReliableOrderedChannel::TryRemoveAckedMessageFromUnacked( uint16 sequence_number,
	                                                               Metrics::MetricsHandler& metrics_handler )
	{
		bool result = false;

		const int32 index = TryGetUnackedMessageIndex( sequence_number );
		if ( index != -1 )
		{
			// Calculate RTT of acked message
			const TimeClock& timeClock = TimeClock::GetInstance();
			const uint32 currentElapsedTime = static_cast< uint32 >( timeClock.GetLocalTimeMilliseconds() );
			const uint32 messageRTT = currentElapsedTime - _unackedMessagesSendTimes[ sequence_number ];
			UpdateRTT( messageRTT );

			// Submit latency and jitter metrics
			const uint32 latency = messageRTT / 2;
			if ( metrics_handler.HasMetric( Metrics::MetricType::LATENCY ) )
			{
				metrics_handler.AddValue( Metrics::MetricType::LATENCY, latency );
			}
			if ( metrics_handler.HasMetric( Metrics::MetricType::JITTER ) )
			{
				metrics_handler.AddValue( Metrics::MetricType::JITTER, latency );
			}

			// Remove message from buffers
			std::unique_ptr< Message > message = RemoveUnackedMessageFromBufferAtIndex( index );

			std::unordered_map< uint16, uint32 >::iterator it = _unackedMessagesSendTimes.find( sequence_number );
			_unackedMessagesSendTimes.erase( it );

			// Release acked message since we no longer need it
			_messageFactory->ReleaseMessage( std::move( message ) );
			result = true;
		}

		return result;
	}

	int32 ReliableOrderedChannel::TryGetUnackedMessageIndex( uint16 sequence_number ) const
	{
		int32 resultIndex = -1;
		uint32 currentIndex = 0;
		for ( std::list< std::unique_ptr< Message > >::const_iterator it = _unackedReliableMessages.cbegin();
		      it != _unackedReliableMessages.cend(); ++it )
		{
			if ( ( *it )->GetHeader().messageSequenceNumber == sequence_number )
			{
				resultIndex = currentIndex;
				break;
			}

			++currentIndex;
		}

		return resultIndex;
	}

	std::unique_ptr< Message > ReliableOrderedChannel::RemoveUnackedMessageFromBufferAtIndex( uint32 index )
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

	const ReliableMessageEntry& ReliableOrderedChannel::GetRemotePeerReliableMessageEntry(
	    uint16 sequence_number ) const
	{
		const uint32 index = GetRollingBufferIndex( sequence_number );
		return _remotePeerReliableMessageEntries[ index ];
	}

	void ReliableOrderedChannel::UpdateRTT( uint32 message_rtt )
	{
		if ( _rttMilliseconds == 0 )
		{
			_rttMilliseconds = message_rtt;
		}
		else
		{
			_rttMilliseconds = Common::AlgorithmUtils::ExponentialMovingAverage( _rttMilliseconds, message_rtt, 10 );
		}

		LOG_INFO( "RTT: %u", _rttMilliseconds );
	}

	float32 ReliableOrderedChannel::GetRetransmissionTimeout() const
	{
		if ( _rttMilliseconds == 0 )
		{
			return INITIAL_TIMEOUT;
		}

		return ( float32 ) _rttMilliseconds / 1000 * 2;
	}

	void ReliableOrderedChannel::SetUnackedMessageSendTime( uint16 sequence_number )
	{
		const TimeClock& timeClock = TimeClock::GetInstance();
		_unackedMessagesSendTimes[ sequence_number ] = static_cast< uint32 >( timeClock.GetLocalTimeMilliseconds() );
	}

	void ReliableOrderedChannel::ClearMessages()
	{
		std::list< std::unique_ptr< Message > >::iterator it = _unackedReliableMessages.begin();
		while ( it != _unackedReliableMessages.end() )
		{
			std::unique_ptr< Message > message( std::move( *it ) );
			_messageFactory->ReleaseMessage( std::move( message ) );

			++it;
		}

		_unackedReliableMessages.clear();
		_unackedReliableMessageTimeouts.clear();
		_unackedMessagesSendTimes.clear();

		it = _unorderedMessagesWaitingForPrevious.begin();
		while ( it != _unorderedMessagesWaitingForPrevious.end() )
		{
			std::unique_ptr< Message > message( std::move( *it ) );
			_messageFactory->ReleaseMessage( std::move( message ) );

			++it;
		}

		_unorderedMessagesWaitingForPrevious.clear();
	}

	uint32 ReliableOrderedChannel::GenerateACKs() const
	{
		// TODO Make the ACK bits be a stream instead of an uint32 so we can easily change it's size
		uint32 acks = 0;
		uint16 firstSequenceNumber = _lastAckedMessageSequenceNumber - 1;
		for ( uint32 i = 0; i < ACK_BITS_SIZE; ++i )
		{
			uint16 currentSequenceNumber = firstSequenceNumber - i;
			const ReliableMessageEntry& reliableMessageEntry =
			    GetRemotePeerReliableMessageEntry( currentSequenceNumber );
			if ( reliableMessageEntry.isAcked && currentSequenceNumber == reliableMessageEntry.sequenceNumber )
			{
				BitwiseUtils::SetBitAtIndex( acks, i );
			}
		}
		return acks;
	}

	void ReliableOrderedChannel::AckReliableMessage( uint16 sequence_number )
	{
		const uint32 index = GetRollingBufferIndex( sequence_number );
		_remotePeerReliableMessageEntries[ index ].sequenceNumber = sequence_number;
		_remotePeerReliableMessageEntries[ index ].isAcked = true;

		_lastAckedMessageSequenceNumber = sequence_number;

		// Set this flag to true so in case this peer does not have any relaible messages pending to be sent, force it
		// so send a reliable packet just to notify of new acked messages from the remote peer.
		_areUnsentACKs = true;
	}

	void ReliableOrderedChannel::ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
	                                          Metrics::MetricsHandler& metrics_handler )
	{
		LOG_INFO( "Last acked from client = %hu", lastAckedMessageSequenceNumber );

		// Check if the last acked is in reliable messages lists
		TryRemoveAckedMessageFromUnacked( lastAckedMessageSequenceNumber, metrics_handler );

		// Check for the rest of acked bits
		const uint16 firstAckSequence = lastAckedMessageSequenceNumber - 1;
		for ( uint32 i = 0; i < ACK_BITS_SIZE; ++i )
		{
			if ( BitwiseUtils::GetBitAtIndex( acks, i ) )
			{
				TryRemoveAckedMessageFromUnacked( firstAckSequence - i, metrics_handler );
			}
		}
	}

	bool ReliableOrderedChannel::IsMessageDuplicated( uint16 sequence_number ) const
	{
		bool result = false;

		const uint32 index = GetRollingBufferIndex( sequence_number );
		if ( _remotePeerReliableMessageEntries[ index ].sequenceNumber == sequence_number &&
		     _remotePeerReliableMessageEntries[ index ].isAcked )
		{
			result = true;
		}

		return result;
	}

	void ReliableOrderedChannel::Update( float32 deltaTime, Metrics::MetricsHandler& metrics_handler )
	{
		// Update unacked message timeouts
		std::list< float32 >::iterator it = _unackedReliableMessageTimeouts.begin();
		while ( it != _unackedReliableMessageTimeouts.end() )
		{
			float32 timeout = *it;
			timeout -= deltaTime;

			if ( timeout <= 0 )
			{
				if ( metrics_handler.HasMetric( Metrics::MetricType::PACKET_LOSS ) )
				{
					metrics_handler.AddValue( Metrics::MetricType::PACKET_LOSS, 1, "LOST" );
				}
				timeout = 0;
			}

			*it = timeout;

			++it;
		}
	}

	void ReliableOrderedChannel::Reset()
	{
		TransmissionChannel::Reset();
		ClearMessages();
		_lastAckedMessageSequenceNumber = 0;
		_nextOrderedMessageSequenceNumber = 1;
		_areUnsentACKs = false;
		_rttMilliseconds = 0;

		for ( uint32 i = 0; i < _reliableMessageEntriesBufferSize; ++i )
		{
			_remotePeerReliableMessageEntries[ i ].Reset();
		}
	}

	ReliableOrderedChannel::~ReliableOrderedChannel()
	{
		ClearMessages();
	}
} // namespace NetLib
