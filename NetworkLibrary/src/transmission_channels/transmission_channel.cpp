#include "transmission_channel.h"

#include <cassert>

#include "communication/message.h"
#include "communication/message_factory.h"

namespace NetLib
{
	TransmissionChannel::TransmissionChannel( TransmissionChannelType type )
	    : _type( type )
	    , _nextMessageSequenceNumber( 1 )
	{
		_unsentMessages.reserve( 5 );
	}

	TransmissionChannel::TransmissionChannel( TransmissionChannel&& other ) noexcept
	    : _type( std::move( other._type ) )
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
		_nextMessageSequenceNumber =
		    std::move( other._nextMessageSequenceNumber ); // unnecessary move, just in case I change that type
		_unsentMessages = std::move( other._unsentMessages );
		_readyToProcessMessages = std::move( other._readyToProcessMessages );
		_processedMessages = std::move( other._processedMessages );
		return *this;
	}

	bool TransmissionChannel::AddMessageToSend( std::unique_ptr< Message > message )
	{
		assert( message != nullptr );

		if ( !IsMessageSuitable( message->GetHeader() ) )
		{
			return false;
		}

		_unsentMessages.push_back( std::move( message ) );
		return true;
	}

	bool TransmissionChannel::ArePendingReadyToProcessMessages() const
	{
		return !_readyToProcessMessages.empty();
	}

	const Message* TransmissionChannel::GetReadyToProcessMessage()
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

	void TransmissionChannel::FreeProcessedMessages()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		while ( !_processedMessages.empty() )
		{
			std::unique_ptr< Message > message( std::move( _processedMessages.front() ) );
			_processedMessages.pop();

			messageFactory.ReleaseMessage( std::move( message ) );
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
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		while ( !_readyToProcessMessages.empty() )
		{
			std::unique_ptr< Message > message( std::move( _readyToProcessMessages.front() ) );
			_readyToProcessMessages.pop();

			messageFactory.ReleaseMessage( std::move( message ) );
		}

		while ( !_processedMessages.empty() )
		{
			std::unique_ptr< Message > message( std::move( _processedMessages.front() ) );
			_processedMessages.pop();

			messageFactory.ReleaseMessage( std::move( message ) );
		}

		for ( std::vector< std::unique_ptr< Message > >::iterator it = _unsentMessages.begin();
		      it != _unsentMessages.end(); ++it )
		{
			std::unique_ptr< Message > message( std::move( *it ) );
			messageFactory.ReleaseMessage( std::move( message ) );
			*it = nullptr;
		}

		_unsentMessages.clear();
	}
} // namespace NetLib
