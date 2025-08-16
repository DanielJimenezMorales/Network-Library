#include "server_hit_registration_system.h"

#include "logger.h"

#include "raycaster.h"
#include "ecs/world.h"
#include "components/collider_2d_component.h"
#include "components/transform_component.h"

#include "shared/components/health_component.h"

#include "server/hit_reg/shot_entry.h"
#include "server/global_components/hit_registration_global_component.h"

#include <vector>

static void SaveCurrentState()
{
}

static void ProcessShotEntry( Engine::ECS::World& world, const ShotEntry& shotEntry )
{
	std::vector< Engine::ECS::GameEntity > entitiesWithColliders =
	    world.GetEntitiesOfBothTypes< Engine::Collider2DComponent, Engine::TransformComponent >();

	if ( entitiesWithColliders.size() > 0 )
	{
		Engine::Raycaster::Ray ray;
		ray.origin = shotEntry.position;
		ray.direction = shotEntry.direction;
		ray.maxDistance = 100.0f;
		Engine::Raycaster::RaycastResult result =
		    Engine::Raycaster::ExecuteRaycast( ray, entitiesWithColliders, shotEntry.shooterEntity );

		if ( result.entity.IsValid() )
		{
			if ( result.entity.HasComponent< HealthComponent >() )
			{
				HealthComponent& healthComponent = result.entity.GetComponent< HealthComponent >();
				healthComponent.ApplyDamage( shotEntry.damage );
				if ( healthComponent.currentHealth == 0 )
				{
					LOG_INFO( "%s, MUERTO", THIS_FUNCTION_NAME );
				}
			}
		}
	}
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
