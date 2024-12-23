#pragma once
#include "NumericTypes.h"

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
			{
			}

			uint32 entityType;
			uint32 id;
			uint32 inGameId;
			uint32 controlledByPeerId;
	};

	class NetworkEntityStorage
	{
		public:
			NetworkEntityStorage() = default;

			bool HasNetworkEntityId( uint32 networkEntityId ) const;
			bool TryGetNetworkEntityFromId( uint32 entityId, NetworkEntityData& gameEntityId );
			void AddNetworkEntity( uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId,
			                       uint32 gameEntityId );
			bool RemoveNetworkEntity( uint32 networkEntityId );
			std::unordered_map< uint32, NetworkEntityData >::const_iterator GetNetworkEntities() const;
			std::unordered_map< uint32, NetworkEntityData >::const_iterator GetPastToEndNetworkEntities() const;

		private:
			std::unordered_map< uint32, NetworkEntityData > _networkEntityIdToDataMap;
	};
} // namespace NetLib
