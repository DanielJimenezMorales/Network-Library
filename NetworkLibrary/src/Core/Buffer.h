#pragma once
#include <cstdint>

namespace NetLib
{
	class Buffer
	{
	public:
		Buffer(uint8_t* data, int size) : _data(data), _size(size)
		{
			_index = 0;
		}

		~Buffer()
		{
		}

		int GetSize() const { return _size; }
		uint8_t* GetData() const { return _data; }
		void Clear();

		void WriteLong(uint64_t value);
		void WriteInteger(uint32_t value);
		void WriteShort(uint16_t value);
		void WriteByte(uint8_t value);
		void WriteFloat(float value);

		uint64_t ReadLong();
		uint32_t ReadInteger();
		uint16_t ReadShort();
		uint8_t ReadByte();
		float ReadFloat();

		void ResetAccessIndex();

	private:
		uint8_t* _data;
		int _size;
		int _index;
	};
}
