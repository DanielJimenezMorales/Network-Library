#include "world.h"

#include <cassert>

#include "logger.h"

#include "vec2f.h"

#include "ecs/game_entity.hpp"

#include "components/transform_component.h"

#include "transform/transform_hierarchy_helper_functions.h"

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
			return CreateGameEntity( prefab_name, position, Vec2f( 1, 0 ) );
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

		bool World::UnsubscribeFromOnEntityCreate( const Common::Delegate< GameEntity& >::SubscriptionHandler& handler )
		{
			return _onEntityCreate.DeleteSubscriber( handler );
		}

		bool World::UnsubscribeFromOnEntityConfigure(
		    const Common::Delegate< GameEntity&, const Prefab& >::SubscriptionHandler& handler )
		{
			return _onEntityConfigure.DeleteSubscriber( handler );
		}

		bool World::UnsubscribeFromOnEntityDestroy(
		    const Common::Delegate< GameEntity& >::SubscriptionHandler& handler )
		{
			return _onEntityDestroy.DeleteSubscriber( handler );
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
			GameEntity newEntity = _entityContainer.CreateGameEntity();

			// Attach components
			if ( !AddComponentsToEntity( archetype, newEntity ) )
			{
				// If components creation failed, we destroy the entity
				_entityContainer.DestroyGameEntity( newEntity.GetId() );
				return GameEntity();
			}

			const TransformComponentProxy transformComponentProxy;
			TransformComponent& newEntityTransform = newEntity.GetComponent< TransformComponent >();
			transformComponentProxy.SetGlobalPosition( newEntityTransform, position );
			transformComponentProxy.SetRotationLookAt( newEntityTransform, look_at_direction );

			// Spawn children entitities, if any
			if ( !prefab->childrenPrefabs.empty() )
			{
				auto childrenPrefabsCit = prefab->childrenPrefabs.cbegin();
				for ( ; childrenPrefabsCit != prefab->childrenPrefabs.cend(); ++childrenPrefabsCit )
				{
					assert( prefab_name != childrenPrefabsCit->name );

					GameEntity childEntity = CreateGameEntity( childrenPrefabsCit->name, position, look_at_direction );
					TransformComponent& childEntityTransformComponent =
					    childEntity.GetComponent< TransformComponent >();
					transformComponentProxy.SetParent( childEntityTransformComponent, childEntity, newEntity );
					transformComponentProxy.SetLocalPosition( childEntityTransformComponent,
					                                          childrenPrefabsCit->localPosition );
					transformComponentProxy.SetLocalRotationAngle( childEntityTransformComponent,
					                                               childrenPrefabsCit->localRotation );
					transformComponentProxy.SetLocalScale( childEntityTransformComponent,
					                                       childrenPrefabsCit->localScale );
				}
			}

			// Configure components
			_onEntityConfigure.Execute( newEntity, *prefab );

			// Call OnEntityCreate
			_onEntityCreate.Execute( newEntity );
			return newEntity;
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
				DestroyInmediateGameEntity( entity );
			}
		}

		void World::DestroyInmediateGameEntity( ECS::GameEntity& entity )
		{
			// Destroy children entities first
			const TransformComponentProxy transformComponentProxy;
			const TransformComponent& entityTransform = entity.GetComponent< TransformComponent >();
			if ( transformComponentProxy.HasChildren( entityTransform ) )
			{
				std::vector< ECS::GameEntity > children = transformComponentProxy.GetChildren( entityTransform );
				auto it = children.begin();
				for ( ; it != children.end(); ++it )
				{
					DestroyInmediateGameEntity( *it );
				}
			}

			// In the really rare case where there could be a weird situation where the children of any child entity is
			// this entity (Cyclic). In that case the issue is within the transform component proxy
			assert( entity.IsValid() );

			_onEntityDestroy.Execute( entity );
			_entityContainer.DestroyGameEntity( entity.GetId() );
		}

	} // namespace ECS
} // namespace Engine
