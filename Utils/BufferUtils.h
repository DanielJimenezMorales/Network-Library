#pragma once
#include <cassert>
#include "../Core/Buffer.h"

class BufferUtils
{
public:
	static void WriteLong(Buffer& buffer, uint64_t value);
	static void WriteInteger(Buffer& buffer, uint32_t value);
	static void WriteShort(Buffer& buffer, uint16_t value);
	static void WriteByte(Buffer& buffer, uint8_t value);

	static uint64_t ReadLong(Buffer& buffer);
	static uint32_t ReadInteger(Buffer& buffer);
	static uint16_t ReadShort(Buffer& buffer);
	static uint8_t ReadByte(Buffer& buffer);
};

