#pragma once
#include "NumericTypes.h"

#include <cassert>

namespace NetLib
{
	class BitwiseUtils
	{
	public:
		static void SetBitAtIndex(uint8& byte, int32 index)
		{
			assert((index >= 0 && index < 8));
			byte |= (1 << index);
		}

		static void SetBitAtIndex(uint32& byte, int32 index)
		{
			assert((index >= 0 && index < 32));
			byte |= (1 << index);
		}

		static bool GetBitAtIndex(uint8 byte, int32 index)
		{
			assert((index >= 0 && index < 8));
			return (byte >> index) & 0x1;
		}

		static bool GetBitAtIndex(uint32 byte, int32 index)
		{
			assert((index >= 0 && index < 32));
			return (byte >> index) & 0x1;
		}
	};
}
