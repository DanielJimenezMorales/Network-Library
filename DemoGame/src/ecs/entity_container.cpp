#include "entity_container.h"

#include "GameEntity.hpp"

namespace ECS
{
	GameEntity EntityContainer::CreateGameEntity()
	{
		entt::entity entityId = _entities.create();
		return GameEntity( entityId, this );
	}

	void EntityContainer::DestroyGameEntity( const GameEntity& gameEntity )
	{
		_entities.destroy( gameEntity._ecsEntityId );
	}

	GameEntity EntityContainer::GetEntityFromId( uint32 id )
	{
		GameEntity result;

		entt::entity entt_id = static_cast< entt::entity >( id );
		if ( _entities.valid( entt_id ) )
		{
			result._ecsEntityId = entt_id;
			result._entityContainer.Set( this );
		}
		return result;
	}
}
