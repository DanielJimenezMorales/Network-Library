#pragma once

namespace NetLib
{
	class NetworkPacket;
	class MessageFactory;
	class Buffer;

	namespace NetworkPacketUtils
	{
		bool ReadNetworkPacket( Buffer& buffer, MessageFactory& message_factory, NetworkPacket& out_packet );
		void CleanPacket( MessageFactory& message_factory, NetworkPacket& packet );
	};
}