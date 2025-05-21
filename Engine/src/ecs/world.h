#pragma once
#include "numeric_types.h"
#include "delegate.h"

#include <vector>
#include <queue>
#include <unordered_map>

#include "ecs/entity_container.h"
#include "ecs/systems_handler.h"
#include "ecs/component_registry.h"
#include "ecs/archetype_registry.h"
#include "ecs/prefab_registry.h"

struct Vec2f;

namespace Engine
{
	namespace ECS
	{
		class GameEntity;

		class World
		{
			public:
				World();

				bool RegisterArchetype( const Archetype& archetype );

				bool RegisterPrefab( Prefab&& prefab );

				template < typename T >
				bool RegisterComponent( const std::string& name );

				void AddSystem( SystemCoordinator* system );

				template < typename T, typename... Params >
				T& AddGlobalComponent( Params&&... params );

				template < typename T >
				T& GetGlobalComponent();
				template < typename T >
				const T& GetGlobalComponent() const;

				// TODO Instead of having all these predefined functions, create one function to to tick the systems.
				// Instead of pass it a compiled enum, pass it something like a string to add flexibility and create as
				// many execution stages as you want.
				void Update( float32 elapsed_time );
				void PreTick( float32 elapsed_time );
				void Tick( float32 elapsed_time );
				void PosTick( float32 elapsed_time );
				void Render( float32 elapsed_time );
				void EndOfFrame();

				GameEntity CreateGameEntity( const std::string& prefab_name, const Vec2f& position );
				GameEntity CreateGameEntity( const std::string& prefab_name, const Vec2f& position,
				                             const Vec2f& look_at_direction );
				void DestroyGameEntity( const GameEntity& entity );
				void DestroyGameEntity( const EntityId entity_id );

				template < typename T >
				GameEntity GetFirstEntityOfType();
				GameEntity GetEntityFromId( uint32 id );

				template < typename T >
				std::vector< GameEntity > GetEntitiesOfType();
				template < typename T >
				const std::vector< GameEntity > GetEntitiesOfType() const;

				template < typename T1, typename T2 >
				const std::vector< GameEntity > GetEntitiesOfBothTypes() const;

				template < typename T >
				T& GetFirstComponentOfType();
				template < typename T >
				const T& GetFirstComponentOfType() const;

				template < typename Functor >
				uint32 SubscribeToOnEntityCreate( Functor&& functor );
				void UnsubscribeFromOnEntityCreate( uint32 id );

				template < typename Functor >
				uint32 SubscribeToOnEntityConfigure( Functor&& functor );
				void UnsubscribeFromOnEntityConfigure( uint32 id );

				template < typename Functor >
				uint32 SubscribeToOnEntityDestroy( Functor&& functor );
				void UnsubscribeFromOnEntityDestroy( uint32 id );

			private:
				void CreatePendingEntities();
				GameEntity SpawnEntity( const std::string& prefab_name, const Vec2f& position,
				                        const Vec2f& look_at_direction );
				bool AddComponentsToEntity( const Archetype& archetype, GameEntity& entity );
				void DestroyPendingEntities();

				EntityContainer _entityContainer;
				SystemsHandler _systemsHandler;

				ComponentRegistry _componentRegistry;
				ArchetypeRegistry _archetype_registry;
				PrefabRegistry _prefab_registry;

				std::queue< EntityId > _entitiesToRemoveRequests;
				std::queue< std::string > _entitiesToCreateRequests;

				Common::Delegate< GameEntity& > _onEntityCreate;
				Common::Delegate< GameEntity&, const Prefab& > _onEntityConfigure;
				Common::Delegate< GameEntity& > _onEntityDestroy;
		};

		template < typename T >
		inline bool World::RegisterComponent( const std::string& name )
		{
			return _componentRegistry.RegisterComponent< T >( name );
		}

		template < typename T, typename... Params >
		inline T& World::AddGlobalComponent( Params&&... params )
		{
			return _entityContainer.AddGlobalComponent< T >( std::forward< Params >( params )... );
		}

		template < typename T >
		inline T& World::GetGlobalComponent()
		{
			return _entityContainer.GetGlobalComponent< T >();
		}

		template < typename T >
		inline const T& World::GetGlobalComponent() const
		{
			return _entityContainer.GetGlobalComponent< T >();
		}

		template < typename T >
		inline GameEntity World::GetFirstEntityOfType()
		{
			return _entityContainer.GetFirstEntityOfType< T >();
		}

		template < typename T >
		inline std::vector< GameEntity > World::GetEntitiesOfType()
		{
			return _entityContainer.GetEntitiesOfType< T >();
		}

		template < typename T >
		inline const std::vector< GameEntity > World::GetEntitiesOfType() const
		{
			return _entityContainer.GetEntitiesOfType< T >();
		}

		template < typename T1, typename T2 >
		inline const std::vector< GameEntity > World::GetEntitiesOfBothTypes() const
		{
			return _entityContainer.GetEntitiesOfBothTypes< T1, T2 >();
		}

		template < typename T >
		inline T& World::GetFirstComponentOfType()
		{
			return _entityContainer.GetFirstComponentOfType< T >();
		}

		template < typename T >
		inline const T& World::GetFirstComponentOfType() const
		{
			return _entityContainer.GetFirstComponentOfType< T >();
		}

		template < typename Functor >
		inline uint32 World::SubscribeToOnEntityCreate( Functor&& functor )
		{
			return _onEntityCreate.AddSubscriber( std::forward< Functor >( functor ) );
		}

		template < typename Functor >
		inline uint32 World::SubscribeToOnEntityConfigure( Functor&& functor )
		{
			return _onEntityConfigure.AddSubscriber( std::forward< Functor >( functor ) );
		}

		template < typename Functor >
		inline uint32 World::SubscribeToOnEntityDestroy( Functor&& functor )
		{
			return _onEntityDestroy.AddSubscriber( std::forward< Functor >( functor ) );
		}
	} // namespace ECS
} // namespace Engine
