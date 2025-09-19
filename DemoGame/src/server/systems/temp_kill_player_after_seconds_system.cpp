#include "temp_kill_player_after_seconds_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"
#include "shared/components/health_component.h"

TempKillPlayerAfterSecondsSystem::TempKillPlayerAfterSecondsSystem()
    : _startCounter( false )
    , _counter( 0.0f )
{
}

void TempKillPlayerAfterSecondsSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	if ( _startCounter )
	{
		_counter += elapsed_time;
		if ( _counter >= 1.f )
		{
			auto entities = world.GetEntitiesOfType< HealthComponent >();
			for ( auto it = entities.begin(); it != entities.end(); ++it )
			{
				HealthComponent& healthComp = it->GetComponent< HealthComponent >();
				healthComp.currentHealth = 0;
			}
			_counter = 0.f;
			_startCounter = false;
		}
	}
}

void TempKillPlayerAfterSecondsSystem::OnPlayerCreated( Engine::ECS::GameEntity& entity )
{
	if ( entity.HasComponent< HealthComponent >() )
	{
		_startCounter = true;
	}
}
