#include "transmission_channel.h"

#include "asserts.h"

#include "communication/message_factory.h"

namespace NetLib
{
	TransmissionChannel::TransmissionChannel( TransmissionChannelType type, MessageFactory* message_factory )
	    : _type( type )
	    , _messageFactory( message_factory )
	    , _nextMessageSequenceNumber( 1 )
	{
		ASSERT( _messageFactory != nullptr, "The Message Factory is nullptr" );
		_unsentMessages.reserve( 5 );
	}

	TransmissionChannel::TransmissionChannel( TransmissionChannel&& other ) noexcept
	    : _type( std::move( other._type ) )
	    , _messageFactory( std::exchange( other._messageFactory, nullptr ) )
	    , // unnecessary move, just in case I change that type
	    _nextMessageSequenceNumber( std::move( other._nextMessageSequenceNumber ) )
	    , // unnecessary move, just in case I change that type
	    _unsentMessages( std::move( other._unsentMessages ) )
	    , _readyToProcessMessages( std::move( other._readyToProcessMessages ) )
	    , _processedMessages( std::move( other._processedMessages ) )
	{
	}

	TransmissionChannel& TransmissionChannel::operator=( TransmissionChannel&& other ) noexcept
	{
		// Release old messages
		ClearMessages();

		// Move data from other to this
		_type = std::move( other._type ); // unnecessary move, just in case I change that type
		_messageFactory = std::exchange( other._messageFactory, nullptr );
		_nextMessageSequenceNumber =
		    std::move( other._nextMessageSequenceNumber ); // unnecessary move, just in case I change that type
		_unsentMessages = std::move( other._unsentMessages );
		_readyToProcessMessages = std::move( other._readyToProcessMessages );
		_processedMessages = std::move( other._processedMessages );
		return *this;
	}

	void TransmissionChannel::FreeProcessedMessages()
	{
		while ( !_processedMessages.empty() )
		{
			std::unique_ptr< Message > message( std::move( _processedMessages.front() ) );
			_processedMessages.pop();

			_messageFactory->ReleaseMessage( std::move( message ) );
		}
	}

	void TransmissionChannel::Reset()
	{
		ClearMessages();
		_nextMessageSequenceNumber = 1;
	}

	TransmissionChannel::~TransmissionChannel()
	{
		ClearMessages();
	}

	void TransmissionChannel::ClearMessages()
	{
		ASSERT( _messageFactory != nullptr, "The Message Factory is nullptr" );

		while ( !_readyToProcessMessages.empty() )
		{
			std::unique_ptr< Message > message( std::move( _readyToProcessMessages.front() ) );
			_readyToProcessMessages.pop();

			_messageFactory->ReleaseMessage( std::move( message ) );
		}

		while ( !_processedMessages.empty() )
		{
			std::unique_ptr< Message > message( std::move( _processedMessages.front() ) );
			_processedMessages.pop();

			_messageFactory->ReleaseMessage( std::move( message ) );
		}

		for ( std::vector< std::unique_ptr< Message > >::iterator it = _unsentMessages.begin();
		      it != _unsentMessages.end(); ++it )
		{
			std::unique_ptr< Message > message( std::move( *it ) );
			_messageFactory->ReleaseMessage( std::move( message ) );
			*it = nullptr;
		}

		_unsentMessages.clear();
	}
} // namespace NetLib
