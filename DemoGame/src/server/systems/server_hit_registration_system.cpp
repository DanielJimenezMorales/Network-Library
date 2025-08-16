#include "server_hit_registration_system.h"

#include "logger.h"

#include "raycaster.h"
#include "ecs/world.h"
#include "components/collider_2d_component.h"
#include "components/transform_component.h"

#include "shared/components/health_component.h"
#include "shared/global_components/network_peer_global_component.h"

#include "server/hit_reg/shot_entry.h"
#include "server/global_components/hit_registration_global_component.h"

#include <vector>

struct RewindableEntitiesSnapshot
{
		std::vector< uint32 > rewindableEntityStates;
};

static void SaveCurrentState( const Engine::ECS::World& world, RewindableEntitiesSnapshot& snapshot )
{
}

static void RestoreCurrentState( Engine::ECS::World& world, const RewindableEntitiesSnapshot& snapshot )
{
}

/// <summary>
/// Check if a shot entry is valid for being processed. This will mean:
/// - The shot's server time has to be greater than the current server time minus the maximum allowed rollback time.
/// </summary>
/// <returns>True if valid, False otherwise</returns>
static bool IsShotEntryValid( const Engine::ECS::World& world,
                              const HitRegistrationGlobalComponent& hit_reg_global_component,
                              const ShotEntry& shot_entry )
{
	bool result = false;

	const NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	const float64 currentServerTime = networkPeerComponent.GetPeerAsServer()->GetServerTime();
	const float64 minAllowedServerTimeToRollback =
	    currentServerTime - static_cast< float64 >( hit_reg_global_component.maxAllowedRollbackTimeSeconds );
	if ( minAllowedServerTimeToRollback <= static_cast< float64 >( shot_entry.serverTime ) )
	{
		result = true;
	}
	else
	{
		LOG_WARNING( "%s: The shot entry has a really old server time. Shot entry server time: %.5f, Oldest allowed "
		             "server time: %.5f",
		             THIS_FUNCTION_NAME, shot_entry.serverTime, minAllowedServerTimeToRollback );
	}

	return result;
}

static void RollbackEntities( Engine::ECS::World& world, float32 serverTime )
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

void ServerHitRegistrationSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	HitRegistrationGlobalComponent& hitRegGlobalComponent =
	    world.GetGlobalComponent< HitRegistrationGlobalComponent >();

	if ( hitRegGlobalComponent.pendingShotEntries.size() > 0 )
	{
		RewindableEntitiesSnapshot snapshot;
		SaveCurrentState( world, snapshot );

		while ( !hitRegGlobalComponent.pendingShotEntries.empty() )
		{
			const ShotEntry shotEntry = hitRegGlobalComponent.pendingShotEntries.front();
			hitRegGlobalComponent.pendingShotEntries.pop();

			if ( IsShotEntryValid( world, hitRegGlobalComponent, shotEntry ) )
			{
				RollbackEntities( world, shotEntry.serverTime );
				ProcessShotEntry( world, shotEntry );
			}
			else
			{
				LOG_WARNING( "%s: Invalid Shot Entry. Skipping it.", THIS_FUNCTION_NAME );
			}
		}

		RestoreCurrentState( world, snapshot );
	}
}
