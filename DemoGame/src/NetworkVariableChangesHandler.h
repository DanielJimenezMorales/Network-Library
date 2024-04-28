#pragma once
#include <cstdint>

class NetworkVariableChangesHandler
{
public:
	void AddChange(float value, uint32_t networkVariableId, uint32_t networkEntityId);
};
