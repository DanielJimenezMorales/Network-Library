#include "client_connection_pipeline.h"

#include "connection/pending_connection.h"
#include "communication/message.h"
#include "communication/message_factory.h"

#include "logger.h"

namespace NetLib
{
	static std::unique_ptr< Message > CreateConnectionChallengeResponseMessage( MessageFactory& message_factory,
	                                                                            uint64 data_prefix )
	{
		LOG_INFO( "%s Creating connection challenge response message for pending connection", THIS_FUNCTION_NAME );

		// Get a connection challenge message
		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::ConnectionChallengeResponse );
		if ( message == nullptr )
		{
			LOG_ERROR(
			    "%s Can't create new Connection Challenge Response Message because the MessageFactory has returned "
			    "a null message",
			    THIS_FUNCTION_NAME );
			return nullptr;
		}

		std::unique_ptr< ConnectionChallengeResponseMessage > connectionChallengeResponseMessage(
		    static_cast< ConnectionChallengeResponseMessage* >( message.release() ) );

		// Set connection challenge fields
		connectionChallengeResponseMessage->prefix = data_prefix;

		return connectionChallengeResponseMessage;
	}

	static void ProcessConnectionChallenge( PendingConnection& pending_connection,
	                                        const ConnectionChallengeMessage& message, MessageFactory& message_factory )
	{
		LOG_INFO( "%s Processing challenge message for pending connection", THIS_FUNCTION_NAME );

		// Check if state is valid for a connection challenge
		if ( pending_connection.GetCurrentState() != PendingConnectionState::ConnectionChallenge &&
		     pending_connection.GetCurrentState() != PendingConnectionState::Initializing )
		{
			LOG_WARNING( "%s Pending connection is not in a valid state to process a connection challenge message. "
			             "Current state: %u",
			             THIS_FUNCTION_NAME, static_cast< uint32 >( pending_connection.GetCurrentState() ) );
			return;
		}

		// Check if client salt matches
		if ( pending_connection.GetClientSalt() != message.clientSalt )
		{
			LOG_ERROR(
			    "%s The generated salt number does not match the server's challenge message salt number. Message "
			    "salt: %llu, pending connection salt: %llu",
			    THIS_FUNCTION_NAME, message.clientSalt, pending_connection.GetClientSalt() );
			return;
		}

		// Update pending connection
		pending_connection.SetServerSalt( message.serverSalt );
		pending_connection.SetCurrentState( PendingConnectionState::ConnectionChallenge );

		// Create connection challenge response
		const uint64 dataPrefix = message.clientSalt ^ message.serverSalt;
		std::unique_ptr< Message > connectionChallengeMessage =
		    CreateConnectionChallengeResponseMessage( message_factory, dataPrefix );
		pending_connection.AddMessage( std::move( connectionChallengeMessage ) );
	}

	static void ProcessConnectionAccepted( PendingConnection& pending_connection,
	                                       const ConnectionAcceptedMessage& message )
	{
		LOG_INFO( "%s Processing connection accepted message for pending connection", THIS_FUNCTION_NAME );

		// Check if state is valid for a connection challenge
		if ( pending_connection.GetCurrentState() != PendingConnectionState::ConnectionChallenge &&
		     pending_connection.GetCurrentState() != PendingConnectionState::Completed )
		{
			LOG_WARNING( "%s Pending connection is not in a valid state to process a connection accepted message. "
			             "Current state: %u",
			             THIS_FUNCTION_NAME, static_cast< uint32 >( pending_connection.GetCurrentState() ) );
			return;
		}

		pending_connection.SetCurrentState( PendingConnectionState::Completed );
		pending_connection.SetId( 0 );
		pending_connection.SetClientSideId( message.clientIndexAssigned );
	}

	static void ProcessConnectionDenied( PendingConnection& pending_connection, const ConnectionDeniedMessage& message )
	{
		LOG_INFO( "%s Processing connection denied message for pending connection", THIS_FUNCTION_NAME );

		pending_connection.SetConnectionDeniedReason( static_cast< ConnectionFailedReasonType >( message.reason ) );
		pending_connection.SetCurrentState( PendingConnectionState::Failed );
	}

	static void ProcessMessage( PendingConnection& pending_connection, const Message* message,
	                            MessageFactory& message_factory )
	{
		const MessageType type = message->GetHeader().type;

		switch ( type )
		{
			case MessageType::ConnectionChallenge:
				{
					ProcessConnectionChallenge( pending_connection,
					                            static_cast< const ConnectionChallengeMessage& >( *message ),
					                            message_factory );
				}
				break;
			case MessageType::ConnectionAccepted:
				{
					ProcessConnectionAccepted( pending_connection,
					                           static_cast< const ConnectionAcceptedMessage& >( *message ) );
				}
				break;
			case MessageType::ConnectionDenied:
				{
					ProcessConnectionDenied( pending_connection,
					                         static_cast< const ConnectionDeniedMessage& >( *message ) );
				}
				break;
			default:
				// TODO Unknown message type in pending connection
				break;
		}
	}

	static std::unique_ptr< Message > CreateConnectionRequestMessage( MessageFactory& message_factory,
	                                                                  uint64 client_salt )
	{
		// Get a connection challenge message
		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::ConnectionRequest );
		if ( message == nullptr )
		{
			LOG_ERROR( "%s Can't create new Connection Request Message because the MessageFactory has returned "
			           "a null message",
			           THIS_FUNCTION_NAME );
			return nullptr;
		}

		std::unique_ptr< ConnectionRequestMessage > connectionRequestMessage(
		    static_cast< ConnectionRequestMessage* >( message.release() ) );

		// Set connection request fields
		connectionRequestMessage->clientSalt = client_salt;

		return connectionRequestMessage;
	}

	static uint64 GenerateClientSaltNumber()
	{
		// TODO Change this for a better generator. rand is not generating a full 64bit integer since its maximum is
		// roughly 32767. I have tried to use mt19937_64 but I think I get a conflict with winsocks and
		// std::uniform_int_distribution
		srand( static_cast< uint32 >( time( NULL ) ) );
		return rand();
	}

	static void AddConnectionRequestMessage( PendingConnection& pending_connection, MessageFactory& message_factory )
	{
		LOG_INFO( "%s Adding connection request message to pending connection", THIS_FUNCTION_NAME );

		if ( !pending_connection.HasClientSaltAssigned() )
		{
			const uint64 clientSalt = GenerateClientSaltNumber();
			pending_connection.SetClientSalt( clientSalt );
		}

		// Create connection request
		std::unique_ptr< Message > connectionRequestMessage =
		    CreateConnectionRequestMessage( message_factory, pending_connection.GetClientSalt() );

		// Add message to pending connection
		pending_connection.AddMessage( std::move( connectionRequestMessage ) );
	}

	void ClientConnectionPipeline::ProcessConnection( PendingConnection& pending_connection,
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

		// Start with new connections
		if ( pending_connection.GetCurrentState() == PendingConnectionState::Initializing )
		{
			AddConnectionRequestMessage( pending_connection, message_factory );
		}

		// TODO Update connection timeouts
	}
} // namespace NetLib
