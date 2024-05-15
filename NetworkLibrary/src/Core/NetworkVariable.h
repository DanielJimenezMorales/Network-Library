#pragma once
#include <cstdint>
#include <type_traits>

namespace NetLib
{
	class NetworkVariableChangesHandler;

	enum NetworkVariableType : uint8_t
	{
		NVT_Invalid = 0,
		NVT_Float = 1
	};

	template <typename T>
	struct NetworkVariableChangeData
	{
	public:
		NetworkVariableChangeData() = default;
		NetworkVariableChangeData(T value, uint32_t networkVariableId, uint32_t networkEntityId) : value(value), networkVariableId(networkVariableId), networkEntityId(networkEntityId) {}
		T value;
		uint32_t networkVariableId;
		uint32_t networkEntityId;
	};

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
		NetworkVariable(NetworkVariableChangesHandler* networkVariableChangesHandler, uint32_t networkEntityId, uint32_t id) :
			_networkVariableChangesHandler(networkVariableChangesHandler), _networkEntityId(networkEntityId), _id(id)
		{
			if (std::is_same<T, float>())
			{
				_type = NVT_Float;
			}

			RegisterVariable();
		};

		~NetworkVariable()
		{
			UnregisterVariable();
		}

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

		NetworkVariableType GetType() const { return _type; };
		uint32_t GetId() const { return _id; };
		uint32_t GetEntityId() const { return _networkEntityId; };

		NetworkVariableChangeData<T> GetChange() const
		{
			return NetworkVariableChangeData<T>(_value, _id, _networkEntityId);
		}

	private:
		T _value;
		uint32_t _id;
		uint32_t _networkEntityId;
		NetworkVariableType _type;

		NetworkVariableChangesHandler* _networkVariableChangesHandler;

		void RegisterVariable() const;
		void UnregisterVariable() const;

		void SetValue(const T& newValue)
		{
			_value = newValue;
		};
	};
}
