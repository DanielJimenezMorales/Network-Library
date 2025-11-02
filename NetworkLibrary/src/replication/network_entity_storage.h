#pragma once
#include "numeric_types.h"

#include "replication/network_entity_communication_callbacks.h"

#include <unordered_map>

namespace NetLib
{
	struct NetworkEntityData
	{
			NetworkEntityData() = default;
			NetworkEntityData( uint32 entityType, uint32 id, uint32 controlledByPeerId )
			    : entityType( entityType )
			    , id( id )
			    , controlledByPeerId( controlledByPeerId )
			    , communicationCallbacks()
			{
			}

			uint32 entityType;
			uint32 id;
			uint32 controlledByPeerId;
			NetworkEntityCommunicationCallbacks communicationCallbacks;
	};

	/// <summary>
	/// This class stores all network entities active within the world.
	/// </summary>
	class NetworkEntityStorage
	{
		public:
			NetworkEntityStorage() = default;
			/// <summary>
			/// Is there a network entity spawned with the same ID?
			/// </summary>
			bool HasNetworkEntityId( uint32 networkEntityId ) const;

			/// <summary>
			/// If found, returns the data associated with a network entity based on its ID.
			/// </summary>
			NetworkEntityData* TryGetNetworkEntityFromId( uint32 entityId );
			NetworkEntityData* AddNetworkEntity( uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId );
			bool RemoveNetworkEntity( uint32 networkEntityId );
			std::unordered_map< uint32, NetworkEntityData >::iterator GetNetworkEntities();
			const std::unordered_map< uint32, NetworkEntityData >& GetNetworkEntitiess() const;
			std::unordered_map< uint32, NetworkEntityData >::iterator GetPastToEndNetworkEntities();

		private:
			std::unordered_map< uint32, NetworkEntityData > _networkEntityIdToDataMap;
	};
} // namespace NetLib
