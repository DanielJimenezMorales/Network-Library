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

	void Buffer::WriteLong(uint64 value)
	{
		assert(_index + 8 <= _size);
		*((uint64*)(_data + _index)) = value;

		_index += 8;
	}

	void Buffer::WriteInteger(uint32 value)
	{
		assert(_index + 4 <= _size);
		*((uint32*)(_data + _index)) = value;

		_index += 4;
	}

	void Buffer::WriteShort(uint16 value)
	{
		assert(_index + 2 <= _size);
		*((uint16*)(_data + _index)) = value;

		_index += 2;
	}

	void Buffer::WriteByte(uint8 value)
	{
		assert(_index + 1 <= _size);
		*((uint8*)(_data + _index)) = value;

		++_index;
	}

	void Buffer::WriteFloat(float32 value)
	{
		assert(_index + 4 <= _size);
		//This memcpy needs to be done using memcpy to keep the bits configuration too
		std::memcpy((_data + _index), &value, sizeof(uint32));
		_index += 4;
	}

	uint64 Buffer::ReadLong()
	{
		assert(_index + 8 <= _size);
		uint64 value;
		value = *((uint64*)(_data + _index));

		_index += 8;
		return value;
	}

	uint32 Buffer::ReadInteger()
	{
		assert(_index + 4 <= _size);
		uint32 value;
		value = *((uint32*)(_data + _index));

		_index += 4;
		return value;
	}

	uint16 Buffer::ReadShort()
	{
		assert(_index + 2 <= _size);
		uint16 value;

		value = *((uint16*)(_data + _index));

		_index += 2;

		return value;
	}

	uint8 Buffer::ReadByte()
	{
		assert(_index + 1 <= _size);
		uint8 value;

		value = *((uint8*)(_data + _index));

		++_index;

		return value;
	}

	float32 Buffer::ReadFloat()
	{
		assert(_index + 4 <= _size);
		float32 value;
		//This memcpy needs to be done using memcpy to recover the bits configuration too
		std::memcpy(&value, (_data + _index), sizeof(float32));

		_index += 4;
		return value;
	}

	void Buffer::ResetAccessIndex()
	{
		_index = 0;
	}
}
