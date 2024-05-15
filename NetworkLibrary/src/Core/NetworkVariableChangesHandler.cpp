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

	void NetworkVariableChangesHandler::ProcessVariableChanges(Buffer& buffer)
	{
		uint16_t numberOfChanges = buffer.ReadShort();

		for (uint16_t i = 0; i < numberOfChanges; ++i)
		{
			uint32_t networkVariableId = buffer.ReadInteger();
			uint32_t networkEntityId = buffer.ReadInteger();

			NetworkVariableType type;
			auto networkVariableTypeIt = _variableIdToTypeMap.find(networkVariableId);
			if (networkVariableTypeIt != _variableIdToTypeMap.cend())
			{
				type = networkVariableTypeIt->second;

				NetworkVariablePairId pairId(networkVariableId, networkEntityId);
				auto networkVariableIt = _floatVariableIdToTypeMap.find(pairId);
				if (networkVariableIt != _floatVariableIdToTypeMap.cend())
				{
					networkVariableIt->second->SetChange(buffer.ReadFloat());
				}
				else
				{
					Common::LOG_ERROR("Network variable not found when trying to assign new value from server");
				}
			}
			else
			{
				Common::LOG_ERROR("We have received a NetworkVariable that is not registered. Aborting current's tick variable processing...");
				return;
			}
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
