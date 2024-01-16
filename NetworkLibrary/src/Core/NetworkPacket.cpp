#include "NetworkPacket.h"
#include "Buffer.h"
#include "MessageFactory.h"
#include "MessageUtils.h"

void NetworkPacketHeader::Write(Buffer& buffer) const
{
	buffer.WriteShort(sequenceNumber);
	buffer.WriteShort(lastAckedSequenceNumber);
	buffer.WriteInteger(ackBits);
}

void NetworkPacketHeader::Read(Buffer& buffer)
{
	sequenceNumber = buffer.ReadShort();
	lastAckedSequenceNumber = buffer.ReadShort();
	ackBits = buffer.ReadInteger();
}

void NetworkPacket::Write(Buffer& buffer) const
{
	header.Write(buffer);

	uint8_t numberOfMessages = messages.size();
	buffer.WriteByte(numberOfMessages);

	std::vector<Message*>::const_iterator iterator = messages.cbegin();
	while (iterator != messages.cend())
	{
		(*iterator)->Write(buffer);
		++iterator;
	}
}

void NetworkPacket::Read(Buffer& buffer)
{
	header.Read(buffer);

	uint8_t numberOfMessages = buffer.ReadByte();

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
	messages.push_back(message);
	return true;
}

std::vector<Message*>::const_iterator NetworkPacket::GetMessages()
{
	return messages.cbegin();
}

void NetworkPacket::ReleaseMessages()
{
	for (int i = GetNumberOfMessages() - 1; i >= 0; --i)
	{
		Message* message = messages[i];
		messages.erase(messages.begin() + i);
		MessageFactory::ReleaseMessage(message);
	}
}

uint32_t NetworkPacket::Size() const
{
	uint32_t packetSize = NetworkPacketHeader::Size();
	packetSize += 1; //We store in 1 byte the number of messages that this packet contains

	std::vector<Message*>::const_iterator iterator = messages.cbegin();
	while (iterator != messages.cend())
	{
		packetSize += (*iterator)->Size();
		++iterator;
	}

	return packetSize;
}
