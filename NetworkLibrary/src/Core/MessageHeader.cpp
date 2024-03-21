#include "MessageHeader.h"
#include "Buffer.h"
#include "BitwiseUtils.h"

void MessageHeader::Write(Buffer& buffer) const
{
	buffer.WriteByte(type);
	buffer.WriteShort(messageSequenceNumber);

	uint8_t flags = 0;
	if (isReliable)
	{
		BitwiseUtils::SetBitAtIndex(flags, 0);
	}

	if (isOrdered)
	{
		BitwiseUtils::SetBitAtIndex(flags, 1);
	}

	buffer.WriteByte(flags);
}

void MessageHeader::Read(Buffer& buffer)
{
	type = static_cast<MessageType>(buffer.ReadByte());
	ReadWithoutHeader(buffer);
}

void MessageHeader::ReadWithoutHeader(Buffer& buffer)
{
	messageSequenceNumber = buffer.ReadShort();
	uint8_t flags = buffer.ReadByte();
	isReliable = BitwiseUtils::GetBitAtIndex(flags, 0);
	isOrdered = BitwiseUtils::GetBitAtIndex(flags, 1);
}
