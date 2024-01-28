#include "Message.h"
#include "Buffer.h"

void ConnectionRequestMessage::Write(Buffer& buffer) const
{
	_header.Write(buffer);
	buffer.WriteLong(clientSalt);
}

void ConnectionRequestMessage::Read(Buffer& buffer)
{
	_header.type = MessageType::ConnectionRequest;
	_header.ReadWithoutHeader(buffer);

	clientSalt = buffer.ReadLong();
}

uint32_t ConnectionRequestMessage::Size() const
{
	return MessageHeader::Size() + sizeof(uint64_t);
}

void ConnectionChallengeMessage::Write(Buffer& buffer) const
{
	_header.Write(buffer);
	buffer.WriteLong(clientSalt);
	buffer.WriteLong(serverSalt);
}

void ConnectionChallengeMessage::Read(Buffer& buffer)
{
	_header.type = MessageType::ConnectionChallenge;
	_header.ReadWithoutHeader(buffer);

	clientSalt = buffer.ReadLong();
	serverSalt = buffer.ReadLong();
}

uint32_t ConnectionChallengeMessage::Size() const
{
	return MessageHeader::Size() + (sizeof(uint64_t) * 2);
}

void ConnectionChallengeResponseMessage::Write(Buffer& buffer) const
{
	_header.Write(buffer);
	buffer.WriteLong(prefix);
}

void ConnectionChallengeResponseMessage::Read(Buffer& buffer)
{
	_header.type = MessageType::ConnectionChallengeResponse;
	_header.ReadWithoutHeader(buffer);

	prefix = buffer.ReadLong();
}

uint32_t ConnectionChallengeResponseMessage::Size() const
{
	return MessageHeader::Size() + sizeof(uint64_t);
}

void ConnectionAcceptedMessage::Write(Buffer& buffer) const
{
	_header.Write(buffer);
	buffer.WriteLong(prefix);
	buffer.WriteShort(clientIndexAssigned);
}

void ConnectionAcceptedMessage::Read(Buffer& buffer)
{
	_header.type = MessageType::ConnectionAccepted;
	_header.ReadWithoutHeader(buffer);

	prefix = buffer.ReadLong();
	clientIndexAssigned = buffer.ReadShort();
}

uint32_t ConnectionAcceptedMessage::Size() const
{
	return MessageHeader::Size() + sizeof(uint64_t) + sizeof(uint16_t);
}

void ConnectionDeniedMessage::Write(Buffer& buffer) const
{
	_header.Write(buffer);
}

void ConnectionDeniedMessage::Read(Buffer& buffer)
{
	_header.type = MessageType::ConnectionDenied;
	_header.ReadWithoutHeader(buffer);
}

uint32_t ConnectionDeniedMessage::Size() const
{
	return MessageHeader::Size();
}

void DisconnectionMessage::Write(Buffer& buffer) const
{
	_header.Write(buffer);
	buffer.WriteLong(prefix);
}

void DisconnectionMessage::Read(Buffer& buffer)
{
	_header.type = MessageType::Disconnection;
	_header.ReadWithoutHeader(buffer);

	prefix = buffer.ReadLong();
}

uint32_t DisconnectionMessage::Size() const
{
	return MessageHeader::Size() + sizeof(uint64_t);
}

void InGameMessage::Write(Buffer& buffer) const
{
	_header.Write(buffer);
	buffer.WriteLong(data);
}

void InGameMessage::Read(Buffer& buffer)
{
	_header.type = MessageType::InGame;
	_header.ReadWithoutHeader(buffer);

	data = buffer.ReadLong();
}

uint32_t InGameMessage::Size() const
{
	return MessageHeader::Size() + sizeof(uint64_t);
}

void InGameResponseMessage::Write(Buffer& buffer) const
{
	_header.Write(buffer);
	buffer.WriteLong(data);
}

void InGameResponseMessage::Read(Buffer& buffer)
{
	_header.type = MessageType::InGameResponse;
	_header.ReadWithoutHeader(buffer);

	data = buffer.ReadLong();
}

uint32_t InGameResponseMessage::Size() const
{
	return MessageHeader::Size() + sizeof(uint64_t);
}
