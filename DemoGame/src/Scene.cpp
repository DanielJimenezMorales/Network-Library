#include "Scene.h"

#include <cassert>

#include "TransformComponent.h"
#include "GameEntity.hpp"
#include "IUpdateSystem.h"
#include "IPreTickSystem.h"
#include "ITickSystem.h"
#include "IPosTickSystem.h"

void Scene::AddSystem( ECS::SystemCoordinator* system )
{
	_systemsHandler.AddSystem( system );
}

void Scene::Update( float32 elapsed_time )
{
	_systemsHandler.TickStage( _entityContainer, elapsed_time, ECS::ExecutionStage::UPDATE );

	auto it = _updateSystems.begin();
	for ( ; it != _updateSystems.end(); ++it )
	{
		( *it )->Update( _entityContainer, elapsed_time );
	}
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

	auto it = _tickSystems.begin();
	for ( ; it != _tickSystems.end(); ++it )
	{
		( *it )->Tick( _entityContainer, elapsed_time );
	}
}

void Scene::PosTick( float32 elapsed_time )
{
	_systemsHandler.TickStage( _entityContainer, elapsed_time, ECS::ExecutionStage::POSTICK );

	auto it = _posTickSystems.begin();
	for ( ; it != _posTickSystems.end(); ++it )
	{
		( *it )->PosTick( _entityContainer, elapsed_time );
	}
}

void Scene::Render( SDL_Renderer* renderer )
{
	_systemsHandler.TickStage( _entityContainer, 1.0f, ECS::ExecutionStage::RENDER );
}

void Scene::AddUpdateSystem( IUpdateSystem* system )
{
	assert( system != nullptr );
	_updateSystems.push_back( system );
}

void Scene::AddPreTickSystem( IPreTickSystem* system )
{
	assert( system != nullptr );
	_preTickSystems.push_back( system );
}

void Scene::AddTickSystem( ITickSystem* system )
{
	assert( system != nullptr );
	_tickSystems.push_back( system );
}

void Scene::AddPosTickSystem( IPosTickSystem* system )
{
	assert( system != nullptr );
	_posTickSystems.push_back( system );
}

GameEntity Scene::CreateGameEntity()
{
	GameEntity newEntity = _entityContainer.CreateGameEntity();
	newEntity.AddComponent< TransformComponent >();

	return newEntity;
}

void Scene::DestroyGameEntity( const GameEntity& entity )
{
	_entityContainer.DestroyGameEntity( entity );
}

GameEntity Scene::GetEntityFromId( uint32 id )
{
	return _entityContainer.GetEntityFromId( id );
}
