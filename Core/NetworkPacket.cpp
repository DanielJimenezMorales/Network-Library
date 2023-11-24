#include "NetworkPacket.h"

void NetworkConnectionRequestPacket::Write(Buffer& buffer) const
{
	buffer.WriteByte(type);
	buffer.WriteLong(clientSalt);
}

void NetworkConnectionChallengePacket::Write(Buffer& buffer) const
{
	buffer.WriteByte(type);
	buffer.WriteLong(clientSalt);
	buffer.WriteLong(serverSalt);
}

void NetworkConnectionChallengeResponsePacket::Write(Buffer& buffer) const
{
	buffer.WriteByte(type);
	buffer.WriteLong(prefix);
}

void NetworkConnectionAcceptedPacket::Write(Buffer& buffer) const
{
	buffer.WriteByte(type);
	buffer.WriteLong(prefix);
	buffer.WriteShort(clientIndexAssigned);
}

void NetworkConnectionDeniedPacket::Write(Buffer& buffer) const
{
	buffer.WriteByte(type);
}

void NetworkDisconnectionPacket::Write(Buffer& buffer) const
{
	buffer.WriteByte(type);
	buffer.WriteLong(prefix);
}
