#include "remote_peers_handler.h"

#include "asserts.h"

#include "core/address.h"
#include "core/peer.h"

namespace NetLib
{
	RemotePeersHandler::RemotePeersHandler()
	    : _maxConnections( 0 )
	    , _isInitialized( false )
	{
	}

	void RemotePeersHandler::Initialize( uint32 max_connections, MessageFactory* message_factory )
	{
		ASSERT( !_isInitialized, "Remote peers handler is already initialized. Deinitialize it first." );
		ASSERT( max_connections > 0, "The maximum number of connections has to be greater than zero." );

		_maxConnections = max_connections;
		_remotePeerSlots.reserve( _maxConnections );
		_remotePeers.reserve( _maxConnections );
		_validRemotePeers.reserve( _maxConnections );

		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			_remotePeerSlots.push_back( false ); // You could use vector.resize in these ones.
			_remotePeers.emplace_back( message_factory );
		}

		_isInitialized = true;
	}

	void RemotePeersHandler::TickRemotePeers( float32 elapsedTime, MessageFactory& message_factory )
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( _remotePeerSlots[ i ] )
			{
				_remotePeers[ i ].Tick( elapsedTime, message_factory );

				// Start the disconnection process for those ones who are inactive
				if ( _remotePeers[ i ].IsInactive() )
				{
					// StartDisconnectingRemotePeer(i, true, ConnectionFailedReasonType::CFR_TIMEOUT);
				}
			}
		}
	}

	bool RemotePeersHandler::AddRemotePeer( const Address& addressInfo, uint16 id, uint64 clientSalt,
	                                        uint64 serverSalt )
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		// TODO Check for every single possible error scenario. Is already connected? Is it maxConnections full? etc.
		// Return error code
		int32 slotIndex = FindFreeRemotePeerSlot();
		if ( slotIndex == -1 )
		{
			return false;
		}

		_remotePeerSlots[ slotIndex ] = true;
		_remotePeers[ slotIndex ].Connect( addressInfo, id, REMOTE_PEER_INACTIVITY_TIME, clientSalt, serverSalt );

		auto it = _validRemotePeers.insert( &( _remotePeers[ slotIndex ] ) );
		assert( it.second ); // If the element was already there it means that we are trying to add it again. ERROR!!
		return true;
	}

	RemotePeersHandlerResult RemotePeersHandler::IsRemotePeerAbleToConnect( const Address& address ) const
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		if ( IsRemotePeerAlreadyConnected( address ) )
		{
			return RemotePeersHandlerResult::RPH_ALREADYEXIST;
		}

		if ( FindFreeRemotePeerSlot() == -1 )
		{
			return RemotePeersHandlerResult::RPH_FULL;
		}

		return RemotePeersHandlerResult::RPH_SUCCESS;
	}

	int32 RemotePeersHandler::FindFreeRemotePeerSlot() const
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		int32 freeIndex = -1;
		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( !_remotePeerSlots[ i ] )
			{
				freeIndex = i;
				break;
			}
		}

		return freeIndex;
	}

	uint32 RemotePeersHandler::GetNumberOfAvailableRemotePeerSlots() const
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		uint32 availableIndex = 0;
		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( !_remotePeerSlots[ i ] )
			{
				++availableIndex;
			}
		}

		return availableIndex;
	}

	RemotePeer* RemotePeersHandler::GetRemotePeerFromAddress( const Address& address )
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		RemotePeer* result = nullptr;
		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( !_remotePeerSlots[ i ] )
			{
				continue;
			}

			if ( _remotePeers[ i ].GetAddress() == address )
			{
				result = &_remotePeers[ i ];
				break;
			}
		}

		return result;
	}

	RemotePeer* RemotePeersHandler::GetRemotePeerFromId( uint32 id )
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		RemotePeer* result = nullptr;
		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( !_remotePeerSlots[ i ] )
			{
				continue;
			}

			if ( _remotePeers[ i ].GetClientIndex() == id )
			{
				result = &_remotePeers[ i ];
				break;
			}
		}

		return result;
	}

	const RemotePeer* RemotePeersHandler::GetRemotePeerFromId( uint32 id ) const
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		const RemotePeer* result = nullptr;
		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( !_remotePeerSlots[ i ] )
			{
				continue;
			}

			if ( _remotePeers[ i ].GetClientIndex() == id )
			{
				result = &_remotePeers[ i ];
				break;
			}
		}

		return result;
	}

	bool RemotePeersHandler::IsRemotePeerAlreadyConnected( const Address& address ) const
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		bool found = false;
		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( !_remotePeerSlots[ i ] )
			{
				continue;
			}

			if ( _remotePeers[ i ].GetAddress() == address )
			{
				found = true;
				break;
			}
		}

		return found;
	}

	bool RemotePeersHandler::DoesRemotePeerIdExist( uint32 id ) const
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		bool result = false;

		if ( GetIndexFromId( id ) != -1 )
		{
			result = true;
		}

		return result;
	}

	std::unordered_set< RemotePeer* >::iterator RemotePeersHandler::GetValidRemotePeersIterator()
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		return _validRemotePeers.begin();
	}

	std::unordered_set< RemotePeer* >::iterator RemotePeersHandler::GetValidRemotePeersPastTheEndIterator()
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		return _validRemotePeers.end();
	}

	void RemotePeersHandler::RemoveAllRemotePeers()
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( _remotePeerSlots[ i ] )
			{
				RemoveRemotePeer( _remotePeers[ i ].GetClientIndex() );
			}
		}
	}

	bool RemotePeersHandler::RemoveRemotePeer( uint32 remotePeerId )
	{
		ASSERT( _isInitialized, "Remote peers handler is not initialized." );

		int32 id = GetIndexFromId( remotePeerId );
		if ( id != -1 )
		{
			_remotePeerSlots[ id ] = false;
			_remotePeers[ id ].Disconnect();

			auto it = _validRemotePeers.find( &( _remotePeers[ id ] ) );
			assert( it != _validRemotePeers.end() ); // If it does not exist in the valid peers and we are trying to
			                                         // delete it, that is an ERROR!!
			_validRemotePeers.erase( it );

			return true;
		}

		return false;
	}

	int32 RemotePeersHandler::GetIndexFromId( uint32 id ) const
	{
		int32 index = -1;
		for ( uint32 i = 0; i < _maxConnections; ++i )
		{
			if ( _remotePeerSlots[ i ] )
			{
				if ( _remotePeers[ i ].GetClientIndex() == id )
				{
					index = i;
					break;
				}
			}
		}

		return index;
	}
} // namespace NetLib
