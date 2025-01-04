#include "Scene.h"

#include <cassert>

#include "GameEntity.hpp"
#include "IPreTickSystem.h"

#include "components/transform_component.h"

Scene::Scene()
    : _entityContainer()
    , _systemsHandler()
{
}

void Scene::AddSystem( ECS::SystemCoordinator* system )
{
	_systemsHandler.AddSystem( system );
}

void Scene::Update( float32 elapsed_time )
{
	_systemsHandler.TickStage( _entityContainer, elapsed_time, ECS::ExecutionStage::UPDATE );
}

void Scene::PreTick( float32 elapsed_time )
{
	_systemsHandler.TickStage( _entityContainer, elapsed_time, ECS::ExecutionStage::PRETICK );

	auto it = _preTickSystems.begin();
	for ( ; it != _preTickSystems.end(); ++it )
	{
		( *it )->PreTick( _entityContainer, elapsed_time );
	}
}

void Scene::Tick( float32 elapsed_time )
{
	_systemsHandler.TickStage( _entityContainer, elapsed_time, ECS::ExecutionStage::TICK );
}

void Scene::PosTick( float32 elapsed_time )
{
	_systemsHandler.TickStage( _entityContainer, elapsed_time, ECS::ExecutionStage::POSTICK );
}

void Scene::Render( float32 elapsed_time )
{
	_systemsHandler.TickStage( _entityContainer, elapsed_time, ECS::ExecutionStage::RENDER );
}

void Scene::EndOfFrame()
{
	DestroyPendingEntities();
}

void Scene::AddPreTickSystem( IPreTickSystem* system )
{
	assert( system != nullptr );
	_preTickSystems.push_back( system );
}

GameEntity Scene::CreateGameEntity()
{
	GameEntity newEntity = _entityContainer.CreateGameEntity();
	newEntity.AddComponent< TransformComponent >();

	return newEntity;
}

void Scene::DestroyGameEntity( const GameEntity& entity )
{
	_entitiesToRemoveRequests.push( entity.GetId() );
}

GameEntity Scene::GetEntityFromId( uint32 id )
{
	return _entityContainer.GetEntityFromId( id );
}

void Scene::DestroyPendingEntities()
{
	while ( !_entitiesToRemoveRequests.empty() )
	{
		const ECS::EntityId entity_id = _entitiesToRemoveRequests.front();
		_entitiesToRemoveRequests.pop();

		_entityContainer.DestroyGameEntity( entity_id );
	}
}
