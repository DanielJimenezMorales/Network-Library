#pragma once
#include "numeric_types.h"

#include <type_traits>

namespace NetLib
{
	typedef uint32 NetworkVariableId;
	static constexpr NetworkVariableId INVALID_NETWORK_VARIABLE_ID = 0;

	class NetworkVariableChangesHandler;

	enum NetworkVariableType : uint8
	{
		NVT_Invalid = 0,
		NVT_float32 = 1
	};

	template <typename T>
	struct NetworkVariableChangeData
	{
	public:
		NetworkVariableChangeData() = default;
		NetworkVariableChangeData(T value, uint32 networkVariableId, uint32 networkEntityId) : value(value), networkVariableId(networkVariableId), networkEntityId(networkEntityId) {}
		T value;
		uint32 networkVariableId;
		uint32 networkEntityId;
	};

	template <typename T>
	struct NetworkVariable
	{
		//Supported types
		static_assert(
			std::is_same<T, uint8>::value ||
			std::is_same<T, uint16>::value ||
			std::is_same<T, uint32>::value ||
			std::is_same<T, uint64>::value ||
			std::is_same<T, int>::value ||
			std::is_same<T, float32>::value
			, "Type not supported for Network variable");

	public:
		NetworkVariable(NetworkVariableChangesHandler* networkVariableChangesHandler, uint32 networkEntityId) :
			_networkVariableChangesHandler(networkVariableChangesHandler), _networkEntityId(networkEntityId), _id(INVALID_NETWORK_VARIABLE_ID)
		{
			if (std::is_same<T, float32>())
			{
				_type = NVT_float32;
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
			return _value;
		};

		T& operator-=(const T& other)
		{
			SetValue(_value - other);
			return _value;
		};

		T& operator+=(const T& other)
		{
			SetValue(_value + other);
			return _value;
		};

		T Get() const { return _value; };
		NetworkVariableType GetType() const { return _type; };
		uint32 GetId() const { return _id; };
		uint32 GetEntityId() const { return _networkEntityId; };

	private:
		T _value;
		uint32 _id;
		uint32 _networkEntityId;
		NetworkVariableType _type;

		NetworkVariableChangesHandler* _networkVariableChangesHandler;

		void RegisterVariable();
		void UnregisterVariable() const;

		void SetValue(const T& newValue)
		{
			_value = newValue;
		};

		void SetId(NetworkVariableId id)
		{
			_id = id;
		}

		NetworkVariableChangeData<T> GetChange() const
		{
			return NetworkVariableChangeData<T>(_value, _id, _networkEntityId);
		}

		friend class NetworkVariableChangesHandler;
	};

	template<typename T>
	inline void NetworkVariable<T>::RegisterVariable()
	{
		_networkVariableChangesHandler->RegisterNetworkVariable(this);
	}

	template<typename T>
	inline void NetworkVariable<T>::UnregisterVariable() const
	{
		_networkVariableChangesHandler->UnregisterNetworkVariable(*this);
	}
}
