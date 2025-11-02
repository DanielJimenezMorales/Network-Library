#include "network_entity_storage.h"

#include <cassert>

namespace NetLib
{
	bool NetworkEntityStorage::HasNetworkEntityId( uint32 networkEntityId ) const
	{
		auto it = _networkEntityIdToDataMap.find( networkEntityId );
		return it != _networkEntityIdToDataMap.cend();
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

	NetworkEntityData* NetworkEntityStorage::AddNetworkEntity( uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId )
	{
		if ( _networkEntityIdToDataMap.find( networkEntityId ) != _networkEntityIdToDataMap.cend() )
		{
			return nullptr;
		}

		_networkEntityIdToDataMap.emplace( networkEntityId,
		                                   NetworkEntityData( entityType, networkEntityId, controlledByPeerId ) );

		return &_networkEntityIdToDataMap[networkEntityId];
	}

	std::unordered_map< uint32, NetworkEntityData >::iterator NetworkEntityStorage::GetNetworkEntities()
	{
		return _networkEntityIdToDataMap.begin();
	}

	const std::unordered_map< uint32, NetworkEntityData >& NetworkEntityStorage::GetNetworkEntitiess() const
	{
		return _networkEntityIdToDataMap;
	}

	std::unordered_map< uint32, NetworkEntityData >::iterator NetworkEntityStorage::GetPastToEndNetworkEntities()
	{
		return _networkEntityIdToDataMap.end();
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
