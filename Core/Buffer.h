#pragma once
#include <cstdint>

class Buffer
{
public:
	Buffer(int size)
	{
		this->_size = size;
		_index = 0;
		_data = new uint8_t[size];
	}

	Buffer(uint8_t* data, int size) : _data(data), _size(size)
	{
		_index = 0;
	}

	~Buffer()
	{
		delete[] _data;
	}

	int GetSize() const { return _size; }
	uint8_t* GetData() const { return _data; }
	void Clear();

	void WriteLong(uint64_t value);
	void WriteInteger(uint32_t value);
	void WriteShort(uint16_t value);
	void WriteByte(uint8_t value);

	uint64_t ReadLong();
	uint32_t ReadInteger();
	uint16_t ReadShort();
	uint8_t ReadByte();

private:
	uint8_t* _data;
	int _size;
	int _index;
};

