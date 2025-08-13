#include "server_hit_registration_system.h"

#include "logger.h"

#include "ecs/world.h"

#include "server/hit_reg/shot_entry.h"
#include "server/global_components/hit_registration_global_component.h"

static void SaveCurrentState()
{
}

static void ProcessShotEntry( Engine::ECS::World& world, const ShotEntry& shotEntry )
{
	LOG_WARNING( "%s", THIS_FUNCTION_NAME );
}

static void RestoreCurrentState()
{
}

void ServerHitRegistrationSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	HitRegistrationGlobalComponent& hitRegGlobalComponent =
	    world.GetGlobalComponent< HitRegistrationGlobalComponent >();

	if ( hitRegGlobalComponent.pendingShotEntries.size() > 0 )
	{
		SaveCurrentState();

		while ( !hitRegGlobalComponent.pendingShotEntries.empty() )
		{
			const ShotEntry shotEntry = hitRegGlobalComponent.pendingShotEntries.front();
			hitRegGlobalComponent.pendingShotEntries.pop();
			ProcessShotEntry( world, shotEntry );
		}

		RestoreCurrentState();
	}
}
