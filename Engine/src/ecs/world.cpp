#include "world.h"

#include <cassert>

#include "logger.h"

#include "Vec2f.h"

#include "ecs/game_entity.hpp"

#include "components/transform_component.h"

namespace Engine
{
	namespace ECS
	{
		World::World()
		    : _entityContainer()
		    , _systemsHandler()
		    , _componentRegistry()
		    , _archetype_registry()
		    , _prefab_registry()
		{
		}

		bool World::RegisterArchetype( const Archetype& archetype )
		{
			return _archetype_registry.RegisterArchetype( archetype );
		}

		bool World::RegisterPrefab( Prefab&& prefab )
		{
			return _prefab_registry.RegisterPrefab( std::forward< Prefab >( prefab ) );
		}

		void World::AddSystem( SystemCoordinator* system )
		{
			_systemsHandler.AddSystem( system );
		}

		void World::InputHandling( float32 elapsed_time )
		{
			_systemsHandler.TickStage( *this, elapsed_time, ExecutionStage::INPUT_HANDLING );
		}

		void World::Update( float32 elapsed_time )
		{
			_systemsHandler.TickStage( *this, elapsed_time, ExecutionStage::UPDATE );
		}

		void World::PreTick( float32 elapsed_time )
		{
			_systemsHandler.TickStage( *this, elapsed_time, ExecutionStage::PRETICK );
		}

		void World::Tick( float32 elapsed_time )
		{
			_systemsHandler.TickStage( *this, elapsed_time, ExecutionStage::TICK );
		}

		void World::PosTick( float32 elapsed_time )
		{
			_systemsHandler.TickStage( *this, elapsed_time, ExecutionStage::POSTICK );
		}

		void World::Render( float32 elapsed_time )
		{
			_systemsHandler.TickStage( *this, elapsed_time, ExecutionStage::RENDER );
		}

		void World::EndOfFrame()
		{
			DestroyPendingEntities();
			CreatePendingEntities();
		}

		GameEntity World::CreateGameEntity( const std::string& prefab_name, const Vec2f& position )
		{
			return CreateGameEntity( prefab_name, position, Vec2f( 0, -1 ) );
		}

		GameEntity World::CreateGameEntity( const std::string& prefab_name, const Vec2f& position,
		                                    const Vec2f& look_at_direction )
		{
			return SpawnEntity( prefab_name, position, look_at_direction );
			//_entitiesToCreateRequests.push( prefab_name );
		}

		void World::DestroyGameEntity( const GameEntity& entity )
		{
			_entitiesToRemoveRequests.push( entity.GetId() );
		}

		void World::DestroyGameEntity( const EntityId entity_id )
		{
			_entitiesToRemoveRequests.push( entity_id );
		}

		GameEntity World::GetEntityFromId( uint32 id )
		{
			return _entityContainer.GetEntityFromId( id );
		}

		void World::UnsubscribeFromOnEntityCreate( uint32 id )
		{
			_onEntityCreate.DeleteSubscriber( id );
		}

		void World::UnsubscribeFromOnEntityConfigure( uint32 id )
		{
			_onEntityConfigure.DeleteSubscriber( id );
		}

		void World::UnsubscribeFromOnEntityDestroy( uint32 id )
		{
			_onEntityDestroy.DeleteSubscriber( id );
		}

		void World::CreatePendingEntities()
		{
			while ( !_entitiesToCreateRequests.empty() )
			{
				const std::string prefab_name = _entitiesToCreateRequests.front();
				_entitiesToCreateRequests.pop();

				// SpawnEntity( prefab_name );
			}
		}

		GameEntity World::SpawnEntity( const std::string& prefab_name, const Vec2f& position,
		                               const Vec2f& look_at_direction )
		{
			// Get prefab
			const Prefab* prefab = _prefab_registry.TryGetPrefab( prefab_name );
			if ( prefab == nullptr )
			{
				LOG_ERROR( "[Scene::SpawnEntity] Can't create game entity, prefab with name %s not found.",
				           prefab_name.c_str() );
				return GameEntity();
			}

			// Get archetype
			Archetype archetype;
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
			new_entity_transform.SetRotationLookAt( look_at_direction );

			// Configure components
			_onEntityConfigure.Execute( new_entity, *prefab );

			// Call OnEntityCreate
			_onEntityCreate.Execute( new_entity );
			return new_entity;
		}

		bool World::AddComponentsToEntity( const Archetype& archetype, GameEntity& entity )
		{
			bool result = true;
			auto cit = archetype.components.cbegin();
			for ( ; cit != archetype.components.cend(); ++cit )
			{
				if ( !_componentRegistry.TryAddComponent( *cit, entity, _entityContainer ) )
				{
					LOG_ERROR(
					    "[Scene::AddComponentsToEntity] Can't add component with name %s because it is not registered.",
					    cit->c_str() );
					result = false;
				}
			}

			return result;
		}

		void World::DestroyPendingEntities()
		{
			while ( !_entitiesToRemoveRequests.empty() )
			{
				const EntityId entity_id = _entitiesToRemoveRequests.front();
				_entitiesToRemoveRequests.pop();

				GameEntity entity = _entityContainer.GetEntityFromId( entity_id );
				_onEntityDestroy.Execute( entity );
				_entityContainer.DestroyGameEntity( entity_id );
			}
		}
	} // namespace ECS
} // namespace Engine
