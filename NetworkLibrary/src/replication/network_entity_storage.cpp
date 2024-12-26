#include "network_entity_storage.h"

#include <cassert>

namespace NetLib
{
	bool NetworkEntityStorage::HasNetworkEntityId( uint32 networkEntityId ) const
	{
		auto it = _networkEntityIdToDataMap.find( networkEntityId );
		return it != _networkEntityIdToDataMap.cend();
	}

	bool NetworkEntityStorage::TryGetNetworkEntityFromId( uint32 entityId, NetworkEntityData& gameEntityId )
	{
		auto it = _networkEntityIdToDataMap.find( entityId );

		if ( it == _networkEntityIdToDataMap.cend() )
		{
			return false;
		}

		gameEntityId = it->second;
		return true;
	}

	NetworkEntityData* NetworkEntityStorage::TryGetNetworkEntityFromId( uint32 entityId )
	{
		NetworkEntityData* result = nullptr;
		auto it = _networkEntityIdToDataMap.find( entityId );

		if ( it != _networkEntityIdToDataMap.cend() )
		{
			result = &it->second;
		}

		return result;
	}

	void NetworkEntityStorage::AddNetworkEntity( uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId,
	                                             uint32 gameEntityId )
	{
		assert( _networkEntityIdToDataMap.find( networkEntityId ) == _networkEntityIdToDataMap.cend() );

		_networkEntityIdToDataMap[ networkEntityId ] =
		    NetworkEntityData( entityType, networkEntityId, gameEntityId, controlledByPeerId );
	}

	NetworkEntityData& NetworkEntityStorage::AddNetworkEntity( uint32 entityType, uint32 networkEntityId,
	                                                           uint32 controlledByPeerId )
	{
		assert( _networkEntityIdToDataMap.find( networkEntityId ) == _networkEntityIdToDataMap.cend() );

		_networkEntityIdToDataMap[ networkEntityId ] =
		    NetworkEntityData( entityType, networkEntityId, controlledByPeerId );

		return _networkEntityIdToDataMap[ networkEntityId ];
	}

	std::unordered_map< uint32, NetworkEntityData >::const_iterator NetworkEntityStorage::GetNetworkEntities() const
	{
		return _networkEntityIdToDataMap.cbegin();
	}

	std::unordered_map< uint32, NetworkEntityData >::const_iterator NetworkEntityStorage::GetPastToEndNetworkEntities()
	    const
	{
		return _networkEntityIdToDataMap.cend();
	}

	bool NetworkEntityStorage::RemoveNetworkEntity( uint32 networkEntityId )
	{
		auto it = _networkEntityIdToDataMap.find( networkEntityId );

		if ( it == _networkEntityIdToDataMap.end() )
		{
			return false;
		}

		_networkEntityIdToDataMap.erase( it );
		return true;
	}
} // namespace NetLib
