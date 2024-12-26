#include "network_entity_factory_registry.h"

#include "replication/i_network_entity_factory.h"

#include <cassert>

namespace NetLib
{
	void NetLib::NetworkEntityFactoryRegistry::RegisterNetworkEntityFactory( INetworkEntityFactory* entityFactory )
	{
		_networkEntityFactory = entityFactory;
	}

	uint32 NetworkEntityFactoryRegistry::CreateNetworkEntity(
	    uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId, float32 posX, float32 posY,
	    NetworkVariableChangesHandler* networkVariableChangesHandler )
	{
		int32 gameEntityId = _networkEntityFactory->CreateNetworkEntityObject(
		    entityType, networkEntityId, controlledByPeerId, posX, posY, networkVariableChangesHandler );
		assert( gameEntityId != -1 );

		return static_cast< uint32 >( gameEntityId );
	}

	void NetworkEntityFactoryRegistry::RemoveNetworkEntity( uint32 inGameId )
	{
		_networkEntityFactory->DestroyNetworkEntityObject( inGameId );
	}
}
