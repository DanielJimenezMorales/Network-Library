#include "NetworkVariableChangesHandler.h"
#include "Logger.h"

namespace NetLib
{
	void NetworkVariableChangesHandler::RegisterNetworkVariable(NetworkVariable<float>* networkVariable)
	{
		_variableIdToTypeMap[networkVariable->GetId()] = networkVariable->GetType();
		NetworkVariablePairId pairId(networkVariable->GetId(), networkVariable->GetEntityId());

		//TODO Check for the right Type. In this case we only have float so there is no issue but in the future when we have more types, consider using a switch
		auto pairIdIt = _floatVariableIdToTypeMap.find(pairId);
		if (pairIdIt == _floatVariableIdToTypeMap.cend())
		{
			_floatVariableIdToTypeMap[pairId] = networkVariable;
		}
		else
		{
			Common::LOG_ERROR("You are trying to registen a network variable that is already registered.");
		}
	}

	void NetworkVariableChangesHandler::UnregisterNetworkVariable(const NetworkVariable<float>& networkVariable)
	{
		NetworkVariablePairId pairId(networkVariable.GetId(), networkVariable.GetEntityId());
		
		auto pairIdIt = _floatVariableIdToTypeMap.find(pairId);
		if (pairIdIt != _floatVariableIdToTypeMap.cend())
		{
			_floatVariableIdToTypeMap.erase(pairIdIt);
		}
		else
		{
			Common::LOG_WARNING("You are trying to unregister a network variable that doesn't exist");
		}
	}

	void NetworkVariableChangesHandler::AddChange(NetworkVariableChangeData<float> variableChange)
	{
		uint32_t networkEntityId = variableChange.networkEntityId;
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
		auto floatVariablesIt = _floatVariableIdToTypeMap.cbegin();

		for (; floatVariablesIt != _floatVariableIdToTypeMap.cend(); ++floatVariablesIt)
		{
			AddChange(floatVariablesIt->second->GetChange());
		}
	}

	const EntityNetworkVariableChanges* NetworkVariableChangesHandler::GetChangesFromEntity(uint32_t networkEntityId)
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
}
