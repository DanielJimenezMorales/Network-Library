#pragma once
#include <cstdint>
#include <unordered_set>
#include <unordered_map>

#include "Buffer.h"

namespace NetLib
{
	enum NetworkVariableType : uint8_t
	{
		NVT_Invalid = 0,
		NVT_Float = 1
	};

	template <typename T>
	struct NetworkVariableChangeData
	{
	public:
		T value;
		uint32_t networkVariableId;
		uint32_t networkEntityId;
	};

	class NetworkVariableChangesHandler
	{
	public:
		void RegisterNetworkVariableType(uint32_t variableId, NetworkVariableType type);
		void AddChange(NetworkVariableChangeData<float> variableChange);

		void GetNetworkVariableChangesFromNetworkEntity(uint32_t networkEntityId, Buffer& buffer) const;
		void Clear();

	private:
		std::unordered_set<NetworkVariableChangeData<float>> _floatChanges;
		std::unordered_map<uint32_t, NetworkVariableType> _variableIdToTypeMap;
	};
}
