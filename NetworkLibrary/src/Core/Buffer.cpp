#include "Buffer.h"
#include <cassert>
#include <cmath>
#include <cstring>

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
		assert(_index + 4 <= _size);
		//This memcpy needs to be done using memcpy to keep the bits configuration too
		std::memcpy((_data + _index), &value, sizeof(uint32_t));
		_index += 4;
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
		assert(_index + 4 <= _size);
		float value;
		//This memcpy needs to be done using memcpy to recover the bits configuration too
		std::memcpy(&value, (_data + _index), sizeof(float));

		_index += 4;
		return value;
	}

	void Buffer::ResetAccessIndex()
	{
		_index = 0;
	}
}
