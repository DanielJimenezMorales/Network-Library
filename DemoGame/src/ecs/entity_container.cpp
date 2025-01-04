#include "entity_container.h"

#include "GameEntity.hpp"

namespace ECS
{
	GameEntity EntityContainer::CreateGameEntity()
	{
		entt::entity entityId = _entities.create();
		return GameEntity( static_cast< EntityId >( entityId ), this );
	}

	void EntityContainer::DestroyGameEntity( const GameEntity& gameEntity )
	{
		_entities.destroy( static_cast< entt::entity >( gameEntity._ecsEntityId ) );
	}

	GameEntity EntityContainer::GetEntityFromId( uint32 id )
	{
		GameEntity result;

		entt::entity entt_id = static_cast< entt::entity >( id );
		if ( _entities.valid( entt_id ) )
		{
			result._ecsEntityId = static_cast< EntityId >( entt_id );
			result._entityContainer.Set( this );
		}
		return result;
	}
}
