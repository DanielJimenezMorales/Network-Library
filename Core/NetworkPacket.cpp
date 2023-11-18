#include "NetworkPacket.h"

void NetworkConnectionRequestPacket::Write(Buffer& buffer) const
{
	BufferUtils::WriteByte(buffer, type);
	BufferUtils::WriteLong(buffer, clientSalt);
}

void NetworkConnectionChallengePacket::Write(Buffer& buffer) const
{
	BufferUtils::WriteByte(buffer, type);
	BufferUtils::WriteLong(buffer, clientSalt);
	BufferUtils::WriteLong(buffer, serverSalt);
}

void NetworkConnectionChallengeResponsePacket::Write(Buffer& buffer) const
{
	BufferUtils::WriteByte(buffer, type);
	BufferUtils::WriteLong(buffer, prefix);
}

void NetworkConnectionAcceptedPacket::Write(Buffer& buffer) const
{
	BufferUtils::WriteByte(buffer, type);
	BufferUtils::WriteLong(buffer, prefix);
	BufferUtils::WriteShort(buffer, clientIndexAssigned);
}

void NetworkConnectionDeniedPacket::Write(Buffer& buffer) const
{
	BufferUtils::WriteByte(buffer, type);
}

void NetworkDisconnectionPacket::Write(Buffer& buffer) const
{
	BufferUtils::WriteByte(buffer, type);
	BufferUtils::WriteLong(buffer, prefix);
}
