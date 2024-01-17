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
	//header.Read(buffer);
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
	//header.Read(buffer);
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
	//header.Read(buffer);
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
	//header.Read(buffer);
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
	//header.Read(buffer);
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
	//header.Read(buffer);
	prefix = buffer.ReadLong();
}

uint32_t DisconnectionMessage::Size() const
{
	return MessageHeader::Size() + sizeof(uint64_t);
}
