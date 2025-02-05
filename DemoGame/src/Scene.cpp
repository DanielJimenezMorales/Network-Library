#include "Scene.h"

#include <cassert>

#include "logger.h"

#include "GameEntity.hpp"
#include "IPreTickSystem.h"
#include "Vec2f.h"

#include "components/transform_component.h"

Scene::Scene()
    : _entityContainer()
    , _systemsHandler()
    , _componentRegistry()
    , _archetype_registry()
    , _prefab_registry()
{
}

bool Scene::RegisterArchetype( const ECS::Archetype& archetype )
{
	return _archetype_registry.RegisterArchetype( archetype );
}

bool Scene::RegisterPrefab( ECS::Prefab&& prefab )
{
	return _prefab_registry.RegisterPrefab( std::forward< ECS::Prefab >( prefab ) );
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
	CreatePendingEntities();
}

void Scene::AddPreTickSystem( IPreTickSystem* system )
{
	assert( system != nullptr );
	_preTickSystems.push_back( system );
}

GameEntity Scene::CreateGameEntity( const std::string& prefab_name, const Vec2f& position )
{
	return SpawnEntity( prefab_name, position );
	//_entitiesToCreateRequests.push( prefab_name );
}

void Scene::DestroyGameEntity( const GameEntity& entity )
{
	_entitiesToRemoveRequests.push( entity.GetId() );
}

void Scene::DestroyGameEntity( const ECS::EntityId entity_id )
{
	_entitiesToRemoveRequests.push( entity_id );
}

GameEntity Scene::GetEntityFromId( uint32 id )
{
	return _entityContainer.GetEntityFromId( id );
}

void Scene::UnsubscribeFromOnEntityCreate( uint32 id )
{
	_onEntityCreate.DeleteSubscriber( id );
}

void Scene::UnsubscribeFromOnEntityConfigure( uint32 id )
{
	_onEntityConfigure.DeleteSubscriber( id );
}

void Scene::UnsubscribeFromOnEntityDestroy( uint32 id )
{
	_onEntityDestroy.DeleteSubscriber( id );
}

void Scene::CreatePendingEntities()
{
	while ( !_entitiesToCreateRequests.empty() )
	{
		const std::string prefab_name = _entitiesToCreateRequests.front();
		_entitiesToCreateRequests.pop();

		// SpawnEntity( prefab_name );
	}
}

GameEntity Scene::SpawnEntity( const std::string& prefab_name, const Vec2f& position )
{
	// Get prefab
	const ECS::Prefab* prefab = _prefab_registry.TryGetPrefab( prefab_name );
	if ( prefab == nullptr )
	{
		LOG_ERROR( "[Scene::SpawnEntity] Can't create game entity, prefab with name %s not found.",
		           prefab_name.c_str() );
		return GameEntity();
	}

	// Get archetype
	ECS::Archetype archetype;
	if ( !_archetype_registry.TryGetArchetype( prefab->archetype, archetype ) )
	{
		LOG_ERROR( "[Scene::SpawnEntity] Can't create game entity, archetype with name "
		           "%s not found.",
		           prefab->archetype.c_str() );
		return GameEntity();
	}

	// Create entity
	GameEntity new_entity = _entityContainer.CreateGameEntity();

	// Attach components
	if ( !AddComponentsToEntity( archetype, new_entity ) )
	{
		// If components creation failed, we destroy the entity
		_entityContainer.DestroyGameEntity( new_entity.GetId() );
		return GameEntity();
	}

	assert( new_entity.HasComponent< TransformComponent >() );

	TransformComponent& new_entity_transform = new_entity.GetComponent< TransformComponent >();
	new_entity_transform.SetPosition( position );

	// Configure components
	_onEntityConfigure.Execute( new_entity, *prefab );

	// Call OnEntityCreate
	_onEntityCreate.Execute( new_entity );
	return new_entity;
}

bool Scene::AddComponentsToEntity( const ECS::Archetype& archetype, GameEntity& entity )
{
	bool result = true;
	auto cit = archetype.components.cbegin();
	for ( ; cit != archetype.components.cend(); ++cit )
	{
		if ( !_componentRegistry.TryAddComponent( *cit, entity, _entityContainer ) )
		{
			LOG_ERROR( "[Scene::AddComponentsToEntity] Can't add component with name %s because it is not registered.",
			           cit->c_str() );
			result = false;
		}
	}

	return result;
}

void Scene::DestroyPendingEntities()
{
	while ( !_entitiesToRemoveRequests.empty() )
	{
		const ECS::EntityId entity_id = _entitiesToRemoveRequests.front();
		_entitiesToRemoveRequests.pop();

		GameEntity entity = _entityContainer.GetEntityFromId( entity_id );
		_onEntityDestroy.Execute( entity );
		_entityContainer.DestroyGameEntity( entity_id );
	}
}
