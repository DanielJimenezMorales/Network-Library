#include "Message.h"
#include "Buffer.h"

void ConnectionRequestMessage::Write(Buffer& buffer) const
{
	header.Write(buffer);
	buffer.WriteLong(clientSalt);
}

void ConnectionRequestMessage::Read(Buffer& buffer)
{
	header.Read(buffer);
	clientSalt = buffer.ReadLong();
}

void ConnectionChallengeMessage::Write(Buffer& buffer) const
{
	header.Write(buffer);
	buffer.WriteLong(clientSalt);
	buffer.WriteLong(serverSalt);
}

void ConnectionChallengeMessage::Read(Buffer& buffer)
{
	header.Read(buffer);
	clientSalt = buffer.ReadLong();
	serverSalt = buffer.ReadLong();
}

void ConnectionChallengeResponseMessage::Write(Buffer& buffer) const
{
	header.Write(buffer);
	buffer.WriteLong(prefix);
}

void ConnectionChallengeResponseMessage::Read(Buffer& buffer)
{
	header.Read(buffer);
	prefix = buffer.ReadLong();
}

void ConnectionAcceptedMessage::Write(Buffer& buffer) const
{
	header.Write(buffer);
	buffer.WriteLong(prefix);
	buffer.WriteShort(clientIndexAssigned);
}

void ConnectionAcceptedMessage::Read(Buffer& buffer)
{
	header.Read(buffer);
	prefix = buffer.ReadLong();
	clientIndexAssigned = buffer.ReadShort();
}

void ConnectionDeniedMessage::Write(Buffer& buffer) const
{
	header.Write(buffer);
}

void ConnectionDeniedMessage::Read(Buffer& buffer)
{
	header.Read(buffer);
}

void DisconnectionMessage::Write(Buffer& buffer) const
{
	header.Write(buffer);
	buffer.WriteLong(prefix);
}

void DisconnectionMessage::Read(Buffer& buffer)
{
	header.Read(buffer);
	prefix = buffer.ReadLong();
}
