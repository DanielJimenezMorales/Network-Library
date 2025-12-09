#include "server_connection_pipeline.h"

#include "logger.h"
#include "asserts.h"

#include "communication/message.h"
#include "communication/message_factory.h"
#include "connection/pending_connection.h"

namespace NetLib
{
	static uint64 GenerateServerSalt()
	{
		// TODO Change this in order to get another random generator that generates 64bit numbers
		srand( static_cast< uint32 >( time( NULL ) ) + 3589 );
		uint64 serverSalt = rand();
		return serverSalt;
	}

	static std::unique_ptr< Message > CreateConnectionChallengeMessage( MessageFactory& message_factory,
	                                                                    uint64 client_salt, uint64 server_salt )
	{
		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::ConnectionChallenge );
		if ( message == nullptr )
		{
			LOG_ERROR( "%s Can't create new Connection Challenge Message because the MessageFactory has returned a "
			           "null message",
			           THIS_FUNCTION_NAME );
			return nullptr;
		}

		std::unique_ptr< ConnectionChallengeMessage > connectionChallengeMessage(
		    static_cast< ConnectionChallengeMessage* >( message.release() ) );
		connectionChallengeMessage->clientSalt = client_salt;
		connectionChallengeMessage->serverSalt = server_salt;

		return connectionChallengeMessage;
	}

	static std::unique_ptr< Message > CreateConnectionDeniedMessage( MessageFactory& message_factory )
	{
		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::ConnectionDenied );
		if ( message == nullptr )
		{
			LOG_ERROR(
			    "%s Can't create new connection denied Message because the MessageFactory has returned a null message",
			    THIS_FUNCTION_NAME );
			return nullptr;
		}

		std::unique_ptr< ConnectionDeniedMessage > connectionDeniedMessage(
		    static_cast< ConnectionDeniedMessage* >( message.release() ) );
		connectionDeniedMessage->reason = 0;

		return connectionDeniedMessage;
	}

	static std::unique_ptr< Message > CreateConnectionAcceptedMessage( MessageFactory& message_factory,
	                                                                   uint64 data_prefix, uint16 id )
	{
		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::ConnectionAccepted );
		if ( message == nullptr )
		{
			LOG_ERROR( "%s Can't create new Connection Accepted Message because the MessageFactory has returned a null "
			           "message",
			           THIS_FUNCTION_NAME );
			return nullptr;
		}

		std::unique_ptr< ConnectionAcceptedMessage > connectionAcceptedPacket(
		    static_cast< ConnectionAcceptedMessage* >( message.release() ) );
		connectionAcceptedPacket->prefix = data_prefix;
		connectionAcceptedPacket->clientIndexAssigned = id;

		return connectionAcceptedPacket;
	}

	static void ProcessConnectionRequest( PendingConnection& pending_connection,
	                                      const ConnectionRequestMessage& message, MessageFactory& message_factory )
	{
		LOG_INFO( "%s Processing connection request message for pending connection", THIS_FUNCTION_NAME );

		std::unique_ptr< Message > outcomeMessage = nullptr;

		// If it is in initializing or connection challenge state, send challenge
		if ( pending_connection.GetCurrentState() == PendingConnectionState::Initializing ||
		     pending_connection.GetCurrentState() == PendingConnectionState::ConnectionChallenge )
		{
			if ( pending_connection.GetCurrentState() == PendingConnectionState::Initializing )
			{
				pending_connection.SetServerSalt( GenerateServerSalt() );
				pending_connection.SetCurrentState( PendingConnectionState::ConnectionChallenge );
			}

			outcomeMessage = CreateConnectionChallengeMessage( message_factory, pending_connection.GetClientSalt(),
			                                                   pending_connection.GetServerSalt() );
		}
		// If it is in completed state, send accepted
		else if ( pending_connection.GetCurrentState() == PendingConnectionState::Completed )
		{
			outcomeMessage = CreateConnectionAcceptedMessage( message_factory, pending_connection.GetDataPrefix(),
			                                                  pending_connection.GetId() );
		}
		// If it is in failed state, send denied
		else if ( pending_connection.GetCurrentState() == PendingConnectionState::Failed )
		{
			outcomeMessage = CreateConnectionDeniedMessage( message_factory );
		}

		ASSERT( outcomeMessage != nullptr, "Message can't be nullptr" );
		pending_connection.AddMessage( std::move( outcomeMessage ) );
	}

	ServerConnectionPipeline::ServerConnectionPipeline()
	    : IConnectionPipeline()
	    , _nextConnectionApprovedId( 1 )
	{
	}

	void ServerConnectionPipeline::ProcessConnection( PendingConnection& pending_connection,
	                                                  MessageFactory& message_factory, float32 elapsed_time )
	{
		// Process pending connections
		bool areMessagesToProcess = true;
		while ( areMessagesToProcess )
		{
			const Message* message = pending_connection.GetPendingReadyToProcessMessage();
			if ( message != nullptr )
			{
				ProcessMessage( pending_connection, message, message_factory );
			}
			else
			{
				areMessagesToProcess = false;
			}
		}
	}

	void ServerConnectionPipeline::ProcessMessage( PendingConnection& pending_connection, const Message* message,
	                                               MessageFactory& message_factory )
	{
		const MessageType type = message->GetHeader().type;

		switch ( type )
		{
			case MessageType::ConnectionAccepted:
				{
					ProcessConnectionRequest( pending_connection,
					                          static_cast< const ConnectionRequestMessage& >( *message ),
					                          message_factory );
				}
				break;
			case MessageType::ConnectionChallengeResponse:
				{
					ProcessConnectionChallengeResponse(
					    pending_connection, static_cast< const ConnectionChallengeResponseMessage& >( *message ),
					    message_factory );
				}
				break;
			default:
				// TODO Unknown message type in pending connection
				break;
		}
	}

	void ServerConnectionPipeline::ProcessConnectionChallengeResponse(
	    PendingConnection& pending_connection, const ConnectionChallengeResponseMessage& message,
	    MessageFactory& message_factory )
	{
		LOG_INFO( "%s Processing challenge response message for pending connection", THIS_FUNCTION_NAME );

		std::unique_ptr< Message > outcomeMessage = nullptr;

		// Check if challenge was calculated correctly
		if ( pending_connection.GetDataPrefix() == message.prefix )
		{
			if ( pending_connection.GetCurrentState() == PendingConnectionState::Failed )
			{
				outcomeMessage = CreateConnectionDeniedMessage( message_factory );
			}
			else
			{
				if ( pending_connection.GetCurrentState() != PendingConnectionState::Completed )
				{
					pending_connection.SetCurrentState( PendingConnectionState::Completed );
					pending_connection.SetId( GenerateNextConnectionApprovedId() );
				}

				outcomeMessage = CreateConnectionAcceptedMessage( message_factory, pending_connection.GetDataPrefix(),
				                                                  pending_connection.GetId() );
			}
		}
		else
		{
			outcomeMessage = CreateConnectionDeniedMessage( message_factory );
		}

		ASSERT( outcomeMessage != nullptr, "Message can't be nullptr" );
		pending_connection.AddMessage( std::move( outcomeMessage ) );
	}

	uint16 ServerConnectionPipeline::GenerateNextConnectionApprovedId()
	{
		const uint16 result = _nextConnectionApprovedId;
		if ( _nextConnectionApprovedId == MAX_UINT16 )
		{
			_nextConnectionApprovedId = 1;
		}
		else
		{
			_nextConnectionApprovedId++;
		}

		return result;
	}
} // namespace NetLib
