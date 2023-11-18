#pragma once
#include <cstdint>

struct Buffer
{
	uint8_t* data;
	int size;
	int index;

	Buffer(int size)
	{
		this->size = size;
		index = 0;
		data = new uint8_t[size];
	}

	~Buffer()
	{
		delete[] data;
	}

	void Clear();
};

