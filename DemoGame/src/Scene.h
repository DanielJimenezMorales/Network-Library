#pragma once
#include "numeric_types.h"
#include "delegate.h"

#include <vector>
#include <queue>
#include <unordered_map>

#include "ecs/entity_container.h"
#include "ecs/systems_handler.h"
#include "ecs/component_registry.h"

#include "archetype_registry.h"
#include "prefab_registry.h"

class GameEntity;
class IPreTickSystem;

struct BaseEntityConfiguration;
class IEntityFactory;

class Scene
{
	public:
		Scene();

		bool RegisterEntityFactory( const std::string& id, IEntityFactory* factory );

		bool RegisterArchetype( const ECS::Archetype& archetype );

		bool RegisterPrefab( const ECS::Prefab& prefab );

		template < typename T >
		bool RegisterComponent( const std::string& name );

		void AddSystem( ECS::SystemCoordinator* system );

		template < typename T, typename... Params >
		T& AddGlobalComponent( Params&&... params );

		void Update( float32 elapsed_time );
		void PreTick( float32 elapsed_time );
		void Tick( float32 elapsed_time );
		void PosTick( float32 elapsed_time );
		void Render( float32 elapsed_time );
		void EndOfFrame();

		void AddPreTickSystem( IPreTickSystem* system );

		GameEntity CreateGameEntity();
		void CreateGameEntity( const std::string& prefab_name );
		GameEntity CreateGameEntity( const std::string& type, const BaseEntityConfiguration* config );
		void DestroyGameEntity( const GameEntity& entity );

		template < typename T >
		GameEntity GetFirstEntityOfType();
		GameEntity GetEntityFromId( uint32 id );

		template < typename Functor >
		uint32 SubscribeToOnEntityCreate( Functor&& functor );
		void UnsubscribeFromOnEntityCreate( uint32 id );

		template < typename Functor >
		uint32 SubscribeToOnEntityDestroy( Functor&& functor );
		void UnsubscribeFromOnEntityDestroy( uint32 id );

	private:
		void CreatePendingEntities();
		void SpawnEntity( const std::string& prefab_name );
		bool AddComponentsToEntity( const ECS::Archetype& archetype, GameEntity& entity );
		void DestroyPendingEntities();

		ECS::EntityContainer _entityContainer;
		ECS::SystemsHandler _systemsHandler;

		ECS::ComponentRegistry _componentRegistry;
		ArchetypeRegistry _archetype_registry;
		PrefabRegistry _prefab_registry;

		std::vector< IPreTickSystem* > _preTickSystems;

		std::queue< ECS::EntityId > _entitiesToRemoveRequests;
		std::queue< std::string > _entitiesToCreateRequests;

		std::unordered_map< std::string, IEntityFactory* > _entityFactories;

		Common::Delegate< GameEntity& > _onEntityCreate;
		Common::Delegate< GameEntity& > _onEntityDestroy;
};

template < typename T >
inline bool Scene::RegisterComponent( const std::string& name )
{
	return _componentRegistry.RegisterComponent< T >( name );
}

template < typename T, typename... Params >
inline T& Scene::AddGlobalComponent( Params&&... params )
{
	return _entityContainer.AddGlobalComponent< T >( std::forward< Params >( params )... );
}

template < typename T >
inline GameEntity Scene::GetFirstEntityOfType()
{
	return _entityContainer.GetFirstEntityOfType< T >();
}

template < typename Functor >
inline uint32 Scene::SubscribeToOnEntityCreate( Functor&& functor )
{
	return _onEntityCreate.AddSubscriber( std::forward< Functor >( functor ) );
}

template < typename Functor >
inline uint32 Scene::SubscribeToOnEntityDestroy( Functor&& functor )
{
	return _onEntityDestroy.AddSubscriber( std::forward< Functor >( functor ) );
}
