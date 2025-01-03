#pragma once
#include "numeric_types.h"

#include <vector>

#include "ecs/entity_container.h"
#include "ecs/systems_handler.h"

class GameEntity;
class IPreTickSystem;
class IPosTickSystem;

class Scene
{
	public:
		Scene();

		void AddSystem( ECS::SystemCoordinator* system );

		void Update( float32 elapsed_time );
		void PreTick( float32 elapsed_time );
		void Tick( float32 elapsed_time );
		void PosTick( float32 elapsed_time );
		void Render( float32 elapsed_time );

		void AddPreTickSystem( IPreTickSystem* system );

		GameEntity CreateGameEntity();
		void DestroyGameEntity( const GameEntity& entity );

		template < typename T >
		GameEntity GetFirstEntityOfType();
		GameEntity GetEntityFromId( uint32 id );

	private:
		ECS::EntityContainer _entityContainer;
		ECS::SystemsHandler _systemsHandler;

		std::vector< IPreTickSystem* > _preTickSystems;
};

template < typename T >
inline GameEntity Scene::GetFirstEntityOfType()
{
	return _entityContainer.GetFirstEntityOfType< T >();
}
