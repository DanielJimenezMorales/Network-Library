#pragma once
#include "numeric_types.h"

#include <SDL_image.h>
#include <vector>

#include "ecs/entity_container.h"
#include "ecs/systems_handler.h"

class GameEntity;
class IUpdateSystem;
class IPreTickSystem;
class ITickSystem;
class IPosTickSystem;

class Scene
{
	public:
		Scene()
		    : _entityContainer()
		    , _systemsHandler()
		    , _updateSystems()
		    , _tickSystems()
		{
		}

		~Scene(){};

		void AddSystem( ECS::SystemCoordinator* system );

		void Update( float32 elapsed_time );
		void PreTick( float32 elapsed_time );
		void Tick( float32 elapsed_time );
		void PosTick( float32 elapsed_time );
		void Render( SDL_Renderer* renderer );

		void AddUpdateSystem( IUpdateSystem* system );
		void AddPreTickSystem( IPreTickSystem* system );
		void AddTickSystem( ITickSystem* system );
		void AddPosTickSystem( IPosTickSystem* system );

		GameEntity CreateGameEntity();
		void DestroyGameEntity( const GameEntity& entity );

		template < typename T >
		GameEntity GetFirstEntityOfType();
		GameEntity GetEntityFromId( uint32 id );

	private:
		ECS::EntityContainer _entityContainer;
		ECS::SystemsHandler _systemsHandler;

		std::vector< IUpdateSystem* > _updateSystems;
		std::vector< IPreTickSystem* > _preTickSystems;
		std::vector< ITickSystem* > _tickSystems;
		std::vector< IPosTickSystem* > _posTickSystems;
};

template < typename T >
inline GameEntity Scene::GetFirstEntityOfType()
{
	return _entityContainer.GetFirstEntityOfType< T >();
}
