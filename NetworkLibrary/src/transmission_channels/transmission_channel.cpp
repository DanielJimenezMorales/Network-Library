#include "transmission_channel.h"

#include <cassert>

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
	    , _sentMessages( std::move( other._sentMessages ) )
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
		_sentMessages = std::move( other._sentMessages );
		_readyToProcessMessages = std::move( other._readyToProcessMessages );
		_processedMessages = std::move( other._processedMessages );
		return *this;
	}

	void TransmissionChannel::AddSentMessage( std::unique_ptr< Message > message )
	{
		_sentMessages.push( std::move( message ) );
	}

	void TransmissionChannel::FreeSentMessages()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		while ( !_sentMessages.empty() )
		{
			std::unique_ptr< Message > message( std::move( _sentMessages.front() ) );
			_sentMessages.pop();

			FreeSentMessage( messageFactory, std::move( message ) );
		}
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

		while ( !_sentMessages.empty() )
		{
			std::unique_ptr< Message > message( std::move( _sentMessages.front() ) );
			_sentMessages.pop();

			messageFactory.ReleaseMessage( std::move( message ) );
		}

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
