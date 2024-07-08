#pragma once
#include "IPreTickSystem.h"
#include "IPosTickSystem.h"

class EntityContainer;

class NetworkSystem : public IPreTickSystem, public IPosTickSystem
{
public:
	NetworkSystem() {}
	void PreTick(EntityContainer& entityContainer, float elapsedTime) const override;
	void PosTick(EntityContainer& entityContainer, float elapsedTime) const override;

private:
	void Server_SpawnRemotePeerConnect(EntityContainer& entityContainer) const;
};
