#pragma once
#include <cstdint>
#include <cassert>

class BitwiseUtils
{
public:
	static void SetBitAtIndex(uint8_t& byte, int index)
	{
		assert((index >= 0 && index < 8));
		byte |= (1 << index);
	}

	static void SetBitAtIndex(uint32_t& byte, int index)
	{
		assert((index >= 0 && index < 32));
		byte |= (1 << index);
	}

	static bool GetBitAtIndex(uint8_t byte, int index)
	{
		assert((index >= 0 && index < 8));
		return (byte >> index) & 0x1;
	}

	static bool GetBitAtIndex(uint32_t byte, int index)
	{
		assert((index >= 0 && index < 32));
		return (byte >> index) & 0x1;
	}
};

