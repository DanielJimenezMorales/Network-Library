#pragma once
#include <cstdint>
#include <type_traits>
#include "Vec2f.h"

template <typename T>
struct NetworkVariable
{
	//Supported types
	static_assert(
		std::is_same<T, uint8_t>::value ||
		std::is_same<T, uint16_t>::value ||
		std::is_same<T, uint32_t>::value ||
		std::is_same<T, uint64_t>::value ||
		std::is_same<T, int>::value ||
		std::is_same<T, float>::value
		, "Type not supported for Network variable");

public:
	NetworkVariable() {};

	T& operator=(const T& other)
	{
		SetValue(other);
	};

	T& operator-=(const T& other)
	{
		SetValue(_value - other);
	};

	T& operator+=(const T& other)
	{
		SetValue(_value + other);
	};

private:
	T _value;
	uint32_t _id;
	uint32_t _networkEntityId;

	void SetValue(const T& newValue)
	{
		
		_value = newValue;
	};
};
