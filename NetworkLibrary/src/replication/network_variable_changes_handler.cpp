#include "network_variable_changes_handler.h"

#include "Logger.h"
#include <cassert>

namespace NetLib
{
	void NetworkVariableChangesHandler::RegisterNetworkVariable(NetworkVariable<float32>* networkVariable)
	{
		assert(networkVariable->GetId() == INVALID_NETWORK_VARIABLE_ID);

		networkVariable->SetId(_nextNetworkVariableId);
		IncrementNextNetworkVariableId();

		_variableIdToTypeMap[networkVariable->GetId()] = networkVariable->GetType();
		NetworkVariablePairId pairId(networkVariable->GetId(), networkVariable->GetEntityId());

		//TODO Check for the right Type. In this case we only have float32 so there is no issue but in the future when we have more types, consider using a switch
		auto pairIdIt = _floatVariableIdToTypeMap.find(pairId);
		if (pairIdIt == _floatVariableIdToTypeMap.cend())
		{
			_floatVariableIdToTypeMap[pairId] = networkVariable;
			LOG_INFO("Registering Network variable. Variable ID: %u, Entity ID: %u", pairId._networkVariableId, pairId._networkEntityId);
		}
		else
		{
			LOG_ERROR("You are trying to register a network variable that is already registered.");
		}
	}

	void NetworkVariableChangesHandler::UnregisterNetworkVariable(const NetworkVariable<float32>& networkVariable)
	{
		NetworkVariablePairId pairId(networkVariable.GetId(), networkVariable.GetEntityId());

		auto pairIdIt = _floatVariableIdToTypeMap.find(pairId);
		if (pairIdIt != _floatVariableIdToTypeMap.cend())
		{
			_floatVariableIdToTypeMap.erase(pairIdIt);
		}
		else
		{
			LOG_WARNING("You are trying to unregister a network variable that doesn't exist");
		}
	}

	void NetworkVariableChangesHandler::AddChange(NetworkVariableChangeData<float32> variableChange)
	{
		uint32 networkEntityId = variableChange.networkEntityId;
		auto entityFoundIt = _networkEntityIdToChangesMap.find(networkEntityId);

		if (entityFoundIt != _networkEntityIdToChangesMap.end())
		{
			entityFoundIt->second.AddChange(variableChange);
		}
		else
		{
			_networkEntityIdToChangesMap[networkEntityId] = EntityNetworkVariableChanges();
			_networkEntityIdToChangesMap[networkEntityId].AddChange(variableChange);
		}
	}

	void NetworkVariableChangesHandler::CollectAllChanges()
	{
		auto float32VariablesIt = _floatVariableIdToTypeMap.cbegin();

		for (; float32VariablesIt != _floatVariableIdToTypeMap.cend(); ++float32VariablesIt)
		{
			AddChange(float32VariablesIt->second->GetChange());
		}
	}

	void NetworkVariableChangesHandler::ProcessVariableChanges(Buffer& buffer)
	{
		uint16 numberOfChanges = buffer.ReadShort();

		for (uint16 i = 0; i < numberOfChanges; ++i)
		{
			uint32 networkVariableId = buffer.ReadInteger();
			uint32 networkEntityId = buffer.ReadInteger();

			NetworkVariableType type;
			auto networkVariableTypeIt = _variableIdToTypeMap.find(networkVariableId);
			if (networkVariableTypeIt != _variableIdToTypeMap.cend())
			{
				type = networkVariableTypeIt->second;

				NetworkVariablePairId pairId(networkVariableId, networkEntityId);
				auto networkVariableIt = _floatVariableIdToTypeMap.find(pairId);
				if (networkVariableIt != _floatVariableIdToTypeMap.cend())
				{
					networkVariableIt->second->SetValue(buffer.ReadFloat());
				}
				else
				{
					LOG_ERROR("Network variable not found when trying to assign new value from server. Var ID: %u, Entity ID: %u", networkVariableId, networkEntityId);
				}
			}
			else
			{
				LOG_ERROR("We have received a NetworkVariable that is not registered. Aborting current's tick variable processing...");
				return;
			}
		}
	}

	const EntityNetworkVariableChanges* NetworkVariableChangesHandler::GetChangesFromEntity(uint32 networkEntityId)
	{
		EntityNetworkVariableChanges* result = nullptr;

		auto entityFoundIt = _networkEntityIdToChangesMap.find(networkEntityId);

		if (entityFoundIt != _networkEntityIdToChangesMap.end())
		{
			result = &(entityFoundIt->second);
		}

		return result;
	}

	void NetworkVariableChangesHandler::Clear()
	{
		_networkEntityIdToChangesMap.clear();
	}

	void NetworkVariableChangesHandler::SetNextNetworkVariableId(NetworkVariableId new_value)
	{
		assert(new_value != INVALID_NETWORK_VARIABLE_ID);

		_nextNetworkVariableId = new_value;
	}

	void NetworkVariableChangesHandler::IncrementNextNetworkVariableId()
	{
		++_nextNetworkVariableId;

		if (_nextNetworkVariableId == INVALID_NETWORK_VARIABLE_ID)
		{
			++_nextNetworkVariableId;
		}
	}
}
