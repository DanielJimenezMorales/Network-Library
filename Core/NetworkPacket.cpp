#include "NetworkPacket.h"
#include "Buffer.h"

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
