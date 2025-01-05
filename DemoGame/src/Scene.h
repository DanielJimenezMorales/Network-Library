#pragma once
#include "numeric_types.h"

#include <vector>
#include <queue>
#include <unordered_map>

#include "ecs/entity_container.h"
#include "ecs/systems_handler.h"

class GameEntity;
class IPreTickSystem;

struct BaseEntityConfiguration;
class IEntityFactory;

class Scene
{
	public:
		Scene();

		bool RegisterEntityFactory( const std::string& id, IEntityFactory* factory );

		void AddSystem( ECS::SystemCoordinator* system );

		void Update( float32 elapsed_time );
		void PreTick( float32 elapsed_time );
		void Tick( float32 elapsed_time );
		void PosTick( float32 elapsed_time );
		void Render( float32 elapsed_time );
		void EndOfFrame();

		void AddPreTickSystem( IPreTickSystem* system );

		GameEntity CreateGameEntity();
		GameEntity CreateGameEntity( const std::string& type, const BaseEntityConfiguration* config );
		void DestroyGameEntity( const GameEntity& entity );

		template < typename T >
		GameEntity GetFirstEntityOfType();
		GameEntity GetEntityFromId( uint32 id );

	private:
		void DestroyPendingEntities();

		ECS::EntityContainer _entityContainer;
		ECS::SystemsHandler _systemsHandler;

		std::vector< IPreTickSystem* > _preTickSystems;

		std::queue< ECS::EntityId > _entitiesToRemoveRequests;

		std::unordered_map< std::string, IEntityFactory* > _entityFactories;
};

template < typename T >
inline GameEntity Scene::GetFirstEntityOfType()
{
	return _entityContainer.GetFirstEntityOfType< T >();
}
