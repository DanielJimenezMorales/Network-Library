#include "EntityContainer.h"
#include "GameEntity.hpp"

GameEntity EntityContainer::CreateGameEntity()
{
    entt::entity entityId = _entities.create();
    return GameEntity(entityId, this);
}

void EntityContainer::DestroyGameEntity(const GameEntity& gameEntity)
{
    _entities.destroy(gameEntity._ecsEntityId);
}
