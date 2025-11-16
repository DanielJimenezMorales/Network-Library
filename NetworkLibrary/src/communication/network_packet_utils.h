#pragma once

namespace NetLib
{
	class NetworkPacket;
	class MessageFactory;

	namespace NetworkPacketUtils
	{
		void CleanPacket( MessageFactory& message_factory, NetworkPacket& packet );
	};
}