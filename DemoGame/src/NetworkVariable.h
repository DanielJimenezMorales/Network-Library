#pragma once
#include <cstdint>
#include <type_traits>

#include "NetworkVariableChangesHandler.h"

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
	NetworkVariable(NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler, uint32_t networkEntityId, uint32_t id) : 
		_networkVariableChangesHandler(networkVariableChangesHandler), _networkEntityId(networkEntityId), _id(id)
	{
		if (std::is_same<T, float>())
		{
			_type = NVT_Float;
		}
	};

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
	NetLib::NetworkVariableType _type;

	NetLib::NetworkVariableChangesHandler* _networkVariableChangesHandler;

	void SetValue(const T& newValue)
	{
		_networkVariableChangesHandler->AddChange(newValue, _id, _networkEntityId);
		_value = newValue;
	};
};
