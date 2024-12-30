#include "Scene.h"

#include <cassert>

#include "TransformComponent.h"
#include "GameEntity.hpp"
#include "IUpdateSystem.h"
#include "IPreTickSystem.h"
#include "ITickSystem.h"
#include "IPosTickSystem.h"

void Scene::Update( float32 elapsedTime )
{
	auto it = _updateSystems.begin();
	for ( ; it != _updateSystems.end(); ++it )
	{
		( *it )->Update( _entityContainer, elapsedTime );
	}
}

void Scene::PreTick( float32 tickElapsedTime )
{
	auto it = _preTickSystems.begin();
	for ( ; it != _preTickSystems.end(); ++it )
	{
		( *it )->PreTick( _entityContainer, tickElapsedTime );
	}
}

void Scene::Tick( float32 tickElapsedTime )
{
	auto it = _tickSystems.begin();
	for ( ; it != _tickSystems.end(); ++it )
	{
		( *it )->Tick( _entityContainer, tickElapsedTime );
	}
}

void Scene::PosTick( float32 tickElapsedTime )
{
	auto it = _posTickSystems.begin();
	for ( ; it != _posTickSystems.end(); ++it )
	{
		( *it )->PosTick( _entityContainer, tickElapsedTime );
	}
}

void Scene::Render( SDL_Renderer* renderer )
{
	_spriteRendererSystem.Render( _entityContainer, renderer );
	_gizmoRendererSystem.Render( _entityContainer, renderer );
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
