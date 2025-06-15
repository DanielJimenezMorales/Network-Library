#include "ping_pong_messages_sender.h"

#include "core/remote_peer.h"

#include "communication/message_factory.h"

namespace NetLib
{
	PingPongMessagesSender::PingPongMessagesSender()
	    : _timeLeftUntilNextPingPongMessage( DEFAULT_PING_PONG_MESSAGE_FREQUENCY_SECONDS )
	    , _pingPongMessageFrequencySeconds( DEFAULT_PING_PONG_MESSAGE_FREQUENCY_SECONDS )
	{
	}

	static std::unique_ptr< PingPongMessage > CreatePingPongMessage()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::PingPong );

		std::unique_ptr< PingPongMessage > pingPongMessage( static_cast< PingPongMessage* >( message.release() ) );

		pingPongMessage->SetOrdered( true );
		pingPongMessage->SetReliability( true );

		LOG_INFO( "PING PONG CREATED" );
		return std::move( pingPongMessage );
	}

	void PingPongMessagesSender::Update( float32 elapsed_time, RemotePeer& remote_peer )
	{
		_timeLeftUntilNextPingPongMessage -= elapsed_time;
		if ( _timeLeftUntilNextPingPongMessage <= 0.f )
		{
			_timeLeftUntilNextPingPongMessage = _pingPongMessageFrequencySeconds;

			std::unique_ptr< PingPongMessage > pingPongMessage = CreatePingPongMessage();
			remote_peer.AddMessage( std::move( pingPongMessage ) );
		}
	}
} // namespace NetLib
