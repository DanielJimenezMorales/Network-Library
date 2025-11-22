#include "message_factory.h"

#include "asserts.h"
#include "logger.h"

namespace NetLib
{
	MessageFactory::MessageFactory( uint32 size )
	{
		_initialSize = size;
		InitializePools();
		_isInitialized = true;
	}

	std::unique_ptr< Message > MessageFactory::LendMessage( MessageType messageType )
	{
		ASSERT( _isInitialized, "MessageFactory is not initialized." );

		std::unique_ptr< Message > message = nullptr;

		std::queue< std::unique_ptr< Message > >* pool = GetPoolFromType( messageType );
		if ( pool == nullptr )
		{
			return nullptr;
		}

		if ( !pool->empty() )
		{
			message = std::move( pool->front() );
			pool->pop();
		}
		else
		{
			LOG_WARNING( "The message pool of type %hhu is empty. Creating a new message... Consider increasing pool "
			             "size. Current init size: %u",
			             messageType, _initialSize );

			message = CreateMessage( messageType );
		}

		return std::move( message );
	}

	void MessageFactory::ReleaseMessage( std::unique_ptr< Message > message )
	{
		ASSERT( _isInitialized, "MessageFactory is not initialized." );
		ASSERT( message != nullptr, "Can't release a nullptr message" );

		message->Reset();

		MessageType messageType = message->GetHeader().type;
		std::queue< std::unique_ptr< Message > >* pool = GetPoolFromType( messageType );
		if ( pool != nullptr )
		{
			pool->push( std::move( message ) );
		}
	}

	MessageFactory::~MessageFactory()
	{
		for ( std::unordered_map< MessageType, std::queue< std::unique_ptr< Message > > >::iterator it =
		          _messagePools.begin();
		      it != _messagePools.end(); ++it )
		{
			ReleasePool( ( *it ).second );
		}

		_messagePools.clear();
	}

	void MessageFactory::InitializePools()
	{
		_messagePools[ MessageType::ConnectionRequest ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::ConnectionRequest ], MessageType::ConnectionRequest );

		_messagePools[ MessageType::ConnectionChallenge ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::ConnectionChallenge ], MessageType::ConnectionChallenge );

		_messagePools[ MessageType::ConnectionChallengeResponse ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::ConnectionChallengeResponse ],
		                MessageType::ConnectionChallengeResponse );

		_messagePools[ MessageType::ConnectionAccepted ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::ConnectionAccepted ], MessageType::ConnectionAccepted );

		_messagePools[ MessageType::ConnectionDenied ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::ConnectionDenied ], MessageType::ConnectionDenied );

		_messagePools[ MessageType::Disconnection ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::Disconnection ], MessageType::Disconnection );

		_messagePools[ MessageType::TimeRequest ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::TimeRequest ], MessageType::TimeRequest );

		_messagePools[ MessageType::TimeResponse ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::TimeResponse ], MessageType::TimeResponse );

		_messagePools[ MessageType::Replication ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::Replication ], MessageType::Replication );

		_messagePools[ MessageType::Inputs ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::Inputs ], MessageType::Inputs );

		_messagePools[ MessageType::PingPong ] = std::queue< std::unique_ptr< Message > >();
		InitializePool( _messagePools[ MessageType::PingPong ], MessageType::PingPong );
	}

	void MessageFactory::InitializePool( std::queue< std::unique_ptr< Message > >& pool, MessageType messageType )
	{
		for ( uint32 i = 0; i < _initialSize; ++i )
		{
			std::unique_ptr< Message > message = CreateMessage( messageType );
			pool.push( std::move( message ) );
		}
	}

	std::queue< std::unique_ptr< Message > >* MessageFactory::GetPoolFromType( MessageType messageType )
	{
		std::queue< std::unique_ptr< Message > >* resultPool = nullptr;

		std::unordered_map< MessageType, std::queue< std::unique_ptr< Message > > >::iterator it =
		    _messagePools.find( messageType );
		if ( it != _messagePools.end() )
		{
			resultPool = &( *it ).second;
		}

		return resultPool;
	}

	std::unique_ptr< Message > MessageFactory::CreateMessage( MessageType messageType )
	{
		std::unique_ptr< Message > resultMessage = nullptr;

		switch ( messageType )
		{
			case MessageType::ConnectionRequest:
				resultMessage = std::make_unique< ConnectionRequestMessage >();
				break;
			case MessageType::ConnectionChallenge:
				resultMessage = std::make_unique< ConnectionChallengeMessage >();
				break;
			case MessageType::ConnectionChallengeResponse:
				resultMessage = std::make_unique< ConnectionChallengeResponseMessage >();
				break;
			case MessageType::ConnectionAccepted:
				resultMessage = std::make_unique< ConnectionAcceptedMessage >();
				break;
			case MessageType::ConnectionDenied:
				resultMessage = std::make_unique< ConnectionDeniedMessage >();
				break;
			case MessageType::Disconnection:
				resultMessage = std::make_unique< DisconnectionMessage >();
				break;
			case MessageType::TimeRequest:
				resultMessage = std::make_unique< TimeRequestMessage >();
				break;
			case MessageType::TimeResponse:
				resultMessage = std::make_unique< TimeResponseMessage >();
				break;
			case MessageType::Replication:
				resultMessage = std::make_unique< ReplicationMessage >();
				break;
			case MessageType::Inputs:
				resultMessage = std::make_unique< InputStateMessage >();
				break;
			case MessageType::PingPong:
				resultMessage = std::make_unique< PingPongMessage >();
				break;
			default:
				LOG_ERROR( "Can't create a new message. Invalid message type" );
				break;
		}

		ASSERT( resultMessage != nullptr, "Message not created properly." );
		ASSERT( resultMessage->GetHeader().type == messageType, "Incorrect message header." );
		return std::move( resultMessage );
	}

	void MessageFactory::ReleasePool( std::queue< std::unique_ptr< Message > >& pool )
	{
		while ( !pool.empty() )
		{
			pool.pop();
		}
	}
} // namespace NetLib
