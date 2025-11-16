#include "network_packet_utils.h"

#include "asserts.h"

#include "communication/network_packet.h"
#include "communication/message_factory.h"

namespace NetLib
{
	void NetworkPacketUtils::CleanPacket( MessageFactory& message_factory, NetworkPacket& packet )
	{
		const uint32 numberOfMessages = packet.GetNumberOfMessages();
		for ( uint32 i = 0; i < numberOfMessages; ++i )
		{
			std::unique_ptr< Message > message = packet.TryGetNextMessage();
			message_factory.ReleaseMessage( std::move( message ) );
		}

		ASSERT( packet.GetNumberOfMessages() == 0, "Network packet still has messages inside." );
	}
} // namespace NetLib
