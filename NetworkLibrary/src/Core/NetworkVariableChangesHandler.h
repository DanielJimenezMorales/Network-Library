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
			return sizeof(uint32_t) + (sizeof(float) + sizeof(uint32_t)) * floatChanges.size();
		}
	};

	struct NetworkVariablePairId
	{
		uint32_t _networkVariableId;
		uint32_t _networkEntityId;

		NetworkVariablePairId() : _networkVariableId(0), _networkEntityId(0) {}
		NetworkVariablePairId(uint32_t networkVariableId, uint32_t networkEntityId) : _networkVariableId(networkVariableId), _networkEntityId(networkEntityId) {}

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
		void RegisterNetworkVariable(NetworkVariable<float>* networkVariable);
		void UnregisterNetworkVariable(const NetworkVariable<float>& networkVariable);
		void AddChange(NetworkVariableChangeData<float> variableChange);

		void CollectAllChanges();

		const EntityNetworkVariableChanges* GetChangesFromEntity(uint32_t networkEntityId);
		void Clear();

	private:
		std::unordered_map<uint32_t, NetworkVariableType> _variableIdToTypeMap;
		std::unordered_map<NetworkVariablePairId, NetworkVariable<float>*, CustomNetworkVariablePairIdHash> _floatVariableIdToTypeMap;
		std::unordered_map<uint32_t, EntityNetworkVariableChanges> _networkEntityIdToChangesMap;
	};
}
