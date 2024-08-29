#pragma once
#include "NumericTypes.h"
#include <deque>
#include <memory>

namespace NetLib
{
	class Buffer;
	class Message;

	struct NetworkPacketHeader
	{
		NetworkPacketHeader() : lastAckedSequenceNumber(0), ackBits(0), channelType(0) {}
		NetworkPacketHeader(uint16 ack, uint32 ack_bits, uint8 channel_type) : lastAckedSequenceNumber(ack), ackBits(ack_bits), channelType(channel_type) {}

		void Write(Buffer& buffer) const;
		void Read(Buffer& buffer);

		static uint32 Size() { return sizeof(uint16) + sizeof(uint32) + sizeof(uint8); };

		void SetACKs(uint32 acks) { ackBits = acks; };
		void SetHeaderLastAcked(uint16 lastAckedMessage) { lastAckedSequenceNumber = lastAckedMessage; };
		void SetChannelType(uint8 type) { channelType = type; };

		uint16 lastAckedSequenceNumber;
		uint32 ackBits;
		uint8 channelType;
	};

	class NetworkPacket
	{
	public:
		//NetworkPacket() : _defaultMTUSizeInBytes(1500) {};
		NetworkPacket();
		NetworkPacket(const NetworkPacket&) = delete;
		NetworkPacket(NetworkPacket&& other) noexcept = default;

		NetworkPacket& operator=(const NetworkPacket&) = delete;
		NetworkPacket& operator=(NetworkPacket&& other) noexcept;

		void Write(Buffer& buffer) const;
		void Read(Buffer& buffer);

		const NetworkPacketHeader& GetHeader() const { return _header; };

		bool AddMessage(std::unique_ptr<Message> message);
		std::unique_ptr<Message> GetMessages();
		uint32 GetNumberOfMessages() const { return _messages.size(); }

		uint32 Size() const;
		uint32 MaxSize() const { return _defaultMTUSizeInBytes; };
		bool CanMessageFit(uint32 sizeOfMessagesInBytes) const;

		void SetHeaderACKs(uint32 acks) { _header.SetACKs(acks); };
		void SetHeaderLastAcked(uint16 lastAckedMessage) { _header.SetHeaderLastAcked(lastAckedMessage); };
		void SetHeaderChannelType(uint8 channelType) { _header.SetChannelType(channelType); };

		~NetworkPacket();

	private:
		const uint32 _defaultMTUSizeInBytes;
		NetworkPacketHeader _header;
		std::deque<std::unique_ptr<Message>> _messages;

		void CleanMessages();
	};
}
