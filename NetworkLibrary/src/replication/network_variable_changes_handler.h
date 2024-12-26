#pragma once
#include "numeric_types.h"

#include <vector>
#include <unordered_map>

#include "core/buffer.h"

#include "replication/network_variable.hpp"

namespace NetLib
{
	struct EntityNetworkVariableChanges
	{
		public:
			uint32 networkEntityId;
			std::vector< NetworkVariableChangeData< float32 > > floatChanges;

			void AddChange( NetworkVariableChangeData< float32 > change ) { floatChanges.push_back( change ); }

			uint32 Size() const
			{
				return ( sizeof( uint32 ) + sizeof( float32 ) + sizeof( uint32 ) ) * floatChanges.size();
			}
	};

	struct NetworkVariablePairId
	{
			NetworkVariableId _networkVariableId;
			uint32 _networkEntityId;

			NetworkVariablePairId() = default;
			NetworkVariablePairId( NetworkVariableId networkVariableId, uint32 networkEntityId )
			    : _networkVariableId( networkVariableId )
			    , _networkEntityId( networkEntityId )
			{
			}

			bool operator==( const NetworkVariablePairId& other ) const
			{
				if ( other._networkVariableId == _networkVariableId && other._networkEntityId == _networkEntityId )
				{
					return true;
				}
				else
				{
					return false;
				}
			}
	};

	class CustomNetworkVariablePairIdHash
	{
		public:
			uint32 operator()( const NetworkVariablePairId& pairId ) const
			{
				return ( pairId._networkVariableId * 31 ) + pairId._networkEntityId;
			}
	};

	class NetworkVariableChangesHandler
	{
		public:
			NetworkVariableChangesHandler()
			    : _nextNetworkVariableId( INVALID_NETWORK_VARIABLE_ID + 1 ){};
			void RegisterNetworkVariable( NetworkVariable< float32 >* networkVariable );
			void UnregisterNetworkVariable( const NetworkVariable< float32 >& networkVariable );
			void AddChange( NetworkVariableChangeData< float32 > variableChange );

			void CollectAllChanges();
			void ProcessVariableChanges( Buffer& buffer );

			const EntityNetworkVariableChanges* GetChangesFromEntity( uint32 networkEntityId );
			void Clear();

			void SetNextNetworkVariableId( NetworkVariableId new_value );

		private:
			NetworkVariableId _nextNetworkVariableId;

			std::unordered_map< NetworkVariableId, NetworkVariableType > _variableIdToTypeMap;
			std::unordered_map< NetworkVariablePairId, NetworkVariable< float32 >*, CustomNetworkVariablePairIdHash >
			    _floatVariableIdToTypeMap;

			// Only for collecting local changes
			std::unordered_map< uint32, EntityNetworkVariableChanges > _networkEntityIdToChangesMap;

			void IncrementNextNetworkVariableId();
	};
} // namespace NetLib
