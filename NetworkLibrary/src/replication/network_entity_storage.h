#pragma once
#include "numeric_types.h"

#include "replication/network_entity_communication_callbacks.h"

#include <unordered_map>

namespace NetLib
{
	struct NetworkEntityData
	{
			NetworkEntityData() = default;
			NetworkEntityData( uint32 entityType, uint32 id, uint32 gameEntityId, uint32 controlledByPeerId )
			    : entityType( entityType )
			    , id( id )
			    , inGameId( gameEntityId )
			    , controlledByPeerId( controlledByPeerId )
			    , communicationCallbacks()
			{
			}

			NetworkEntityData( uint32 entityType, uint32 id, uint32 controlledByPeerId )
			    : entityType( entityType )
			    , id( id )
			    , inGameId( 0 )
			    , controlledByPeerId( controlledByPeerId )
			    , communicationCallbacks()
			{
			}

			uint32 entityType;
			uint32 id;
			uint32 inGameId;
			uint32 controlledByPeerId;
			NetworkEntityCommunicationCallbacks communicationCallbacks;
	};

	class NetworkEntityStorage
	{
		public:
			NetworkEntityStorage() = default;

			bool HasNetworkEntityId( uint32 networkEntityId ) const;
			bool TryGetNetworkEntityFromId( uint32 entityId, NetworkEntityData& gameEntityId );
			NetworkEntityData* TryGetNetworkEntityFromId( uint32 entityId );
			void AddNetworkEntity( uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId,
			                       uint32 gameEntityId );
			NetworkEntityData& AddNetworkEntity( uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId );
			bool RemoveNetworkEntity( uint32 networkEntityId );
			std::unordered_map< uint32, NetworkEntityData >::const_iterator GetNetworkEntities() const;
			std::unordered_map< uint32, NetworkEntityData >::const_iterator GetPastToEndNetworkEntities() const;

		private:
			std::unordered_map< uint32, NetworkEntityData > _networkEntityIdToDataMap;
	};
} // namespace NetLib
