#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "Buffer.h"
#include "NetworkVariable.h"

namespace NetLib
{
	struct EntityNetworkVariableChanges
	{
	public:
		uint32_t networkEntityId;
		std::vector<NetworkVariableChangeData<float>> floatChanges;

		void AddChange(NetworkVariableChangeData<float> change)
		{
			floatChanges.push_back(change);
		}

		size_t Size() const
		{
			return (sizeof(uint32_t) + sizeof(float) + sizeof(uint32_t)) * floatChanges.size();
		}
	};

	struct NetworkVariablePairId
	{
		NetworkVariableId _networkVariableId;
		uint32_t _networkEntityId;

		NetworkVariablePairId() = default;
		NetworkVariablePairId(NetworkVariableId networkVariableId, uint32_t networkEntityId) : _networkVariableId(networkVariableId), _networkEntityId(networkEntityId) {}

		bool operator==(const NetworkVariablePairId& other) const
		{
			if (other._networkVariableId == _networkVariableId && other._networkEntityId == _networkEntityId)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	class CustomNetworkVariablePairIdHash
	{
	public:
		size_t operator()(const NetworkVariablePairId& pairId) const
		{
			return (pairId._networkVariableId * 31) + pairId._networkEntityId;
		}
	};

	class NetworkVariableChangesHandler
	{
	public:
		NetworkVariableChangesHandler() : _nextNetworkVariableId(INVALID_NETWORK_VARIABLE_ID + 1) {};
		void RegisterNetworkVariable(NetworkVariable<float>* networkVariable);
		void UnregisterNetworkVariable(const NetworkVariable<float>& networkVariable);
		void AddChange(NetworkVariableChangeData<float> variableChange);

		void CollectAllChanges();
		void ProcessVariableChanges(Buffer& buffer);

		const EntityNetworkVariableChanges* GetChangesFromEntity(uint32_t networkEntityId);
		void Clear();

		void SetNextNetworkVariableId(NetworkVariableId new_value);

	private:
		NetworkVariableId _nextNetworkVariableId;

		std::unordered_map<NetworkVariableId, NetworkVariableType> _variableIdToTypeMap;
		std::unordered_map<NetworkVariablePairId, NetworkVariable<float>*, CustomNetworkVariablePairIdHash> _floatVariableIdToTypeMap;

		//Only for collecting local changes
		std::unordered_map<uint32_t, EntityNetworkVariableChanges> _networkEntityIdToChangesMap;

		void IncrementNextNetworkVariableId();
	};
}
