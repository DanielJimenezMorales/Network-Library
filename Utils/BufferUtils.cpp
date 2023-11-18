#include "BufferUtils.h"

void BufferUtils::WriteLong(Buffer& buffer, uint64_t value)
{
	assert(buffer.index + 8 <= buffer.size);
	*((uint64_t*)(buffer.data + buffer.index)) = value;

	buffer.index += 8;
}

void BufferUtils::WriteInteger(Buffer& buffer, uint32_t value)
{
	assert(buffer.index + 4 <= buffer.size);
	*((uint32_t*)(buffer.data + buffer.index)) = value;

	buffer.index += 4;
}

void BufferUtils::WriteShort(Buffer& buffer, uint16_t value)
{
	assert(buffer.index + 2 <= buffer.size);
	*((uint16_t*)(buffer.data + buffer.index)) = value;

	buffer.index += 2;
}

void BufferUtils::WriteByte(Buffer& buffer, uint8_t value)
{
	assert(buffer.index + 1 <= buffer.size);
	*((uint8_t*)(buffer.data + buffer.index)) = value;

	++buffer.index;
}

uint64_t BufferUtils::ReadLong(Buffer& buffer)
{
	assert(buffer.index + 8 <= buffer.size);
	uint64_t value;
	value = *((uint64_t*)(buffer.data + buffer.index));

	buffer.index += 8;
	return value;
}

uint32_t BufferUtils::ReadInteger(Buffer& buffer)
{
	assert(buffer.index + 4 <= buffer.size);
	uint32_t value;
	value = *((uint32_t*)(buffer.data + buffer.index));

	buffer.index += 4;
	return value;
}

uint16_t BufferUtils::ReadShort(Buffer& buffer)
{
	assert(buffer.index + 2 <= buffer.size);
	uint16_t value;

	value = *((uint16_t*)(buffer.data + buffer.index));

	buffer.index += 2;

	return value;
}

uint8_t BufferUtils::ReadByte(Buffer& buffer)
{
	assert(buffer.index + 1 <= buffer.size);
	uint8_t value;

	value = *((uint8_t*)(buffer.data + buffer.index));

	++buffer.index;

	return value;
}