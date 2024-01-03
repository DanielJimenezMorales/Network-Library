#include "MessageHeader.h"
#include "Buffer.h"

void MessageHeader::Write(Buffer& buffer) const
{
	buffer.WriteByte(type);
}

void MessageHeader::Read(Buffer& buffer)
{
	type = static_cast<MessageType>(buffer.ReadByte());
}
