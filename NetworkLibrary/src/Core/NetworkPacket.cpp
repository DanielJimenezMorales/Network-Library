#include <cassert>

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

NetworkPacket::NetworkPacket(uint16_t packetSequenceNumber) : _header(packetSequenceNumber, 0, 0)
{
	_messages.reserve(5);
}

void NetworkPacket::Write(Buffer& buffer) const
{
	_header.Write(buffer);

	uint8_t numberOfMessages = _messages.size();
	buffer.WriteByte(numberOfMessages);

	std::vector<Message*>::const_iterator iterator = _messages.cbegin();
	while (iterator != _messages.cend())
	{
		(*iterator)->Write(buffer);
		++iterator;
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

std::vector<Message*>::const_iterator NetworkPacket::GetMessages()
{
	return _messages.cbegin();
}

void NetworkPacket::ReleaseMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);
	
	for (int i = GetNumberOfMessages() - 1; i >= 0; --i)
	{
		Message* message = _messages[i];
		_messages.erase(_messages.begin() + i);
		messageFactory->ReleaseMessage(message);
	}
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
