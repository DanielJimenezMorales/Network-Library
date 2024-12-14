#include "network_packet.h"
#include <cassert>

#include "Buffer.h"

#include "communication/message.h"
#include "communication/message_utils.h"
#include "communication/message_factory.h"

namespace NetLib
{
	void NetworkPacketHeader::Write(Buffer& buffer) const
	{
		buffer.WriteShort(lastAckedSequenceNumber);
		buffer.WriteInteger(ackBits);
		buffer.WriteByte(channelType);
	}

	void NetworkPacketHeader::Read(Buffer& buffer)
	{
		lastAckedSequenceNumber = buffer.ReadShort();
		ackBits = buffer.ReadInteger();
		channelType = buffer.ReadByte();
	}

	NetworkPacket::NetworkPacket() : _header(0, 0, 0), _defaultMTUSizeInBytes(1500)
	{
	}

	NetworkPacket& NetworkPacket::operator=(NetworkPacket&& other) noexcept
	{
		CleanMessages();

		_header = std::move(other._header);
		_messages = std::move(other._messages);

		return *this;
	}

	void NetworkPacket::Write(Buffer& buffer) const
	{
		_header.Write(buffer);

		uint8 numberOfMessages = _messages.size();
		buffer.WriteByte(numberOfMessages);

		for (std::deque<std::unique_ptr<Message>>::const_iterator cit = _messages.cbegin(); cit != _messages.cend(); ++cit)
		{
			const Message* message = (*cit).get();
			message->Write(buffer);
		}
	}

	void NetworkPacket::Read(Buffer& buffer)
	{
		_header.Read(buffer);

		uint8 numberOfMessages = buffer.ReadByte();

		for (uint32 i = 0; i < numberOfMessages; ++i)
		{
			std::unique_ptr<Message> message = MessageUtils::ReadMessage(buffer);
			if (message != nullptr)
			{
				AddMessage(std::move(message));
			}
		}
	}

	bool NetworkPacket::AddMessage(std::unique_ptr<Message> message)
	{
		_messages.push_back(std::move(message));
		return true;
	}

	std::unique_ptr<Message> NetworkPacket::GetMessages()
	{
		if (GetNumberOfMessages() == 0)
		{
			return nullptr;
		}

		std::unique_ptr<Message> message = std::move(_messages.front());
		_messages.pop_front();
		return std::move(message);
	}

	uint32 NetworkPacket::Size() const
	{
		uint32 packetSize = NetworkPacketHeader::Size();
		packetSize += 1; //We store in 1 byte the number of messages that this packet contains

		std::deque<std::unique_ptr<Message>>::const_iterator iterator = _messages.cbegin();
		while (iterator != _messages.cend())
		{
			packetSize += (*iterator)->Size();
			++iterator;
		}

		return packetSize;
	}

	bool NetworkPacket::CanMessageFit(uint32 sizeOfMessagesInBytes) const
	{
		return (sizeOfMessagesInBytes + Size() < MaxSize());
	}

	NetworkPacket::~NetworkPacket()
	{
		CleanMessages();
	}

	void NetworkPacket::CleanMessages()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::deque<std::unique_ptr<Message>>::iterator it = _messages.begin();
		for (it; it != _messages.end(); ++it)
		{
			std::unique_ptr<Message> message = std::move(*it);
			messageFactory.ReleaseMessage(std::move(message));
		}

		_messages.clear();
	}
}
