#pragma once
#include "IPreTickSystem.h"
#include "IPosTickSystem.h"
#include <cstdint>

class NetworkSystem
    : public IPreTickSystem
    , public IPosTickSystem
{
	public:
		NetworkSystem() {}
		void PreTick( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const override;
		void PosTick( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const override;

	private:
		void Server_SpawnRemotePeerConnect( ECS::EntityContainer& entityContainer, uint32 remotePeerId ) const;
		// TODO Add another callback for despawn entities
};
