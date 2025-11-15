#pragma once

namespace NetLib
{
	class Buffer;
	class NetworkPacket;
	class MessageFactory;

	class NetworkPacketProcessor
	{
		public:
			NetworkPacketProcessor();

			void Initialize( MessageFactory* message_factory );

			bool ReadPacket( Buffer& buffer, NetworkPacket& out_packet ) const;
			bool WritePacket( const NetworkPacket& packet, Buffer& out_buffer ) const;
			void CleanPacket( NetworkPacket& packet ) const;

		private:
			MessageFactory* _messageFactory;
			bool _isInitialized;
	};
}