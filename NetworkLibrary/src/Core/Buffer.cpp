#include "Buffer.h"
#include <cassert>
#include <cmath>

namespace NetLib
{
	void Buffer::Clear()
	{
		_index = 0;
	}

	void Buffer::WriteLong(uint64_t value)
	{
		assert(_index + 8 <= _size);
		*((uint64_t*)(_data + _index)) = value;

		_index += 8;
	}

	void Buffer::WriteInteger(uint32_t value)
	{
		assert(_index + 4 <= _size);
		*((uint32_t*)(_data + _index)) = value;

		_index += 4;
	}

	void Buffer::WriteShort(uint16_t value)
	{
		assert(_index + 2 <= _size);
		*((uint16_t*)(_data + _index)) = value;

		_index += 2;
	}

	void Buffer::WriteByte(uint8_t value)
	{
		assert(_index + 1 <= _size);
		*((uint8_t*)(_data + _index)) = value;

		++_index;
	}

	void Buffer::WriteFloat(float value)
	{
		WriteInteger(value);
	}

	uint64_t Buffer::ReadLong()
	{
		assert(_index + 8 <= _size);
		uint64_t value;
		value = *((uint64_t*)(_data + _index));

		_index += 8;
		return value;
	}

	uint32_t Buffer::ReadInteger()
	{
		assert(_index + 4 <= _size);
		uint32_t value;
		value = *((uint32_t*)(_data + _index));

		_index += 4;
		return value;
	}

	uint16_t Buffer::ReadShort()
	{
		assert(_index + 2 <= _size);
		uint16_t value;

		value = *((uint16_t*)(_data + _index));

		_index += 2;

		return value;
	}

	uint8_t Buffer::ReadByte()
	{
		assert(_index + 1 <= _size);
		uint8_t value;

		value = *((uint8_t*)(_data + _index));

		++_index;

		return value;
	}

	float Buffer::ReadFloat()
	{
		return ReadInteger();
	}

	void Buffer::ResetAccessIndex()
	{
		_index = 0;
	}
}
