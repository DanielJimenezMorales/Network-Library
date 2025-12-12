#include "server_connection_pipeline.h"

#include "logger.h"
#include "asserts.h"

#include "communication/message.h"
#include "communication/message_factory.h"
#include "connection/pending_connection.h"
#include "core/peer.h"

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
		LOG_INFO( "%s Creating connection challenge message for pending connection", THIS_FUNCTION_NAME );

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

	static std::unique_ptr< Message > CreateConnectionDeniedMessage( MessageFactory& message_factory,
	                                                                 ConnectionFailedReasonType reason )
	{
		LOG_INFO( "%s Creating connection denied message for pending connection", THIS_FUNCTION_NAME );

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
		connectionDeniedMessage->reason = reason;

		return connectionDeniedMessage;
	}

	static std::unique_ptr< Message > CreateConnectionAcceptedMessage( MessageFactory& message_factory,
	                                                                   uint64 data_prefix, uint16 id )
	{
		LOG_INFO( "%s Creating connection accepted message for pending connection", THIS_FUNCTION_NAME );

		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::ConnectionAccepted );
		if ( message == nullptr )
		{
			LOG_ERROR( "%s Can't create new Connection Accepted Message because the MessageFactory has returned a null "
			           "message",
			           THIS_FUNCTION_NAME );
			return nullptr;
		}

		std::unique_ptr< ConnectionAcceptedMessage > connectionAcceptedMessage(
		    static_cast< ConnectionAcceptedMessage* >( message.release() ) );
		connectionAcceptedMessage->prefix = data_prefix;
		connectionAcceptedMessage->clientIndexAssigned = id;

		return connectionAcceptedMessage;
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
				pending_connection.SetClientSalt( message.clientSalt );
				pending_connection.SetServerSalt( GenerateServerSalt() );
				pending_connection.GenerateDataPrefix();
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
			outcomeMessage =
			    CreateConnectionDeniedMessage( message_factory, pending_connection.GetConnectionDeniedReason() );
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
			case MessageType::ConnectionRequest:
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
				outcomeMessage =
				    CreateConnectionDeniedMessage( message_factory, pending_connection.GetConnectionDeniedReason() );
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
			pending_connection.SetCurrentState( PendingConnectionState::Failed );
			pending_connection.SetConnectionDeniedReason( ConnectionFailedReasonType::CFR_WRONG_CHALLENGE_RESPONSE );
			outcomeMessage =
			    CreateConnectionDeniedMessage( message_factory, pending_connection.GetConnectionDeniedReason() );
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
