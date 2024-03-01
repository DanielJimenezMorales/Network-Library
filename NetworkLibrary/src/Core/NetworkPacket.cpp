#include <cassert>

#include "NetworkPacket.h"
#include "Buffer.h"
#include "Message.h"
#include "MessageUtils.h"

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
	_messages.reserve(5);
}

void NetworkPacket::Write(Buffer& buffer) const
{
	_header.Write(buffer);

	uint8_t numberOfMessages = _messages.size();
	buffer.WriteByte(numberOfMessages);

	for (std::vector<Message*>::const_iterator cit = _messages.cbegin(); cit != _messages.cend(); ++cit)
	{
		const Message* message = *cit;
		message->Write(buffer);
	}
}

void NetworkPacket::Read(Buffer& buffer)
{
	_header.Read(buffer);

	uint8_t numberOfMessages = buffer.ReadByte();
	_messages.reserve(numberOfMessages);

	Message* message = nullptr;
	for (unsigned int i = 0; i < numberOfMessages; ++i)
	{
		MessageUtils::ReadMessage(buffer, &message);
		if (message != nullptr)
		{
			AddMessage(message);
		}
	}
}

bool NetworkPacket::AddMessage(Message* message)
{
	_messages.push_back(message);
	return true;
}

std::vector<Message*>::iterator NetworkPacket::GetMessages()
{
	return _messages.begin();
}

uint32_t NetworkPacket::Size() const
{
	uint32_t packetSize = NetworkPacketHeader::Size();
	packetSize += 1; //We store in 1 byte the number of messages that this packet contains

	std::vector<Message*>::const_iterator iterator = _messages.cbegin();
	while (iterator != _messages.cend())
	{
		packetSize += (*iterator)->Size();
		++iterator;
	}

	return packetSize;
}

bool NetworkPacket::CanMessageFit(unsigned int sizeOfMessagesInBytes) const
{
	return (sizeOfMessagesInBytes + Size() < MaxSize());
}

NetworkPacket::~NetworkPacket()
{
	_messages.clear();
}
