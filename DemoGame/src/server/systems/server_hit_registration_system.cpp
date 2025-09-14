#include "server_hit_registration_system.h"

#include "logger.h"
#include "AlgorithmUtils.h"

#include "raycaster.h"
#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/collider_2d_component.h"
#include "components/transform_component.h"

#include "read_only_transform_component_proxy.h"
#include "transform_component_proxy.h"

#include "shared/components/health_component.h"
#include "shared/components/network_entity_component.h"
#include "shared/global_components/network_peer_global_component.h"

#include "server/hit_reg/shot_entry.h"
#include "server/components/server_transform_history_component.h"
#include "server/global_components/hit_registration_global_component.h"

#include <vector>
#include <cassert>

struct RewindableEntitiesSnapshot
{
		struct Entry
		{
				Engine::ECS::EntityId entityId;
				Vec2f position;
				float32 rotationAngle;
		};

		std::vector< Entry > rewindableEntityStates;
};

static void SaveCurrentState(
    /*TODO Add const here since we won't modify it. Although I get an error when I do it. Investigate it*/ Engine::ECS::
        World& world,
    RewindableEntitiesSnapshot& snapshot )
{
	// TODO Filter by entities not only with NetworkEntityComponent but also with Collider component
	const std::vector< Engine::ECS::GameEntity > entitiesWithNetworkComponent =
	    world.GetEntitiesOfType< NetworkEntityComponent >();

	auto cit = entitiesWithNetworkComponent.cbegin();
	for ( ; cit != entitiesWithNetworkComponent.cend(); ++cit )
	{
		if ( cit->HasComponent< Engine::Collider2DComponent >() )
		{
			Engine::ReadOnlyTransformComponentProxy transformComponent( *cit );
			RewindableEntitiesSnapshot::Entry entry;
			entry.entityId = cit->GetId();
			entry.position = transformComponent.GetGlobalPosition();
			entry.rotationAngle = transformComponent.GetGlobalRotationAngle();
			snapshot.rewindableEntityStates.push_back( entry );
		}
	}
}

static void RestoreCurrentState( Engine::ECS::World& world, const RewindableEntitiesSnapshot& snapshot )
{
	auto cit = snapshot.rewindableEntityStates.cbegin();
	for ( ; cit != snapshot.rewindableEntityStates.cend(); ++cit )
	{
		Engine::ECS::GameEntity entity = world.GetEntityFromId( cit->entityId );
		assert( entity.IsValid() );

		Engine::TransformComponentProxy transformComponent( entity );
		transformComponent.SetGlobalPosition( cit->position );
		transformComponent.SetGlobalRotationAngle( cit->rotationAngle );
	}
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

static bool IsTimeBetweenLeftAndRight( const ServerTransformHistoryComponent& transform_history_component,
                                       float32 target_time, int32 left, int32 right )
{
	bool isGreaterThanLeft = false;
	if ( left >= 0 )
	{
		if ( transform_history_component.serverTimeBuffer[ left ] <= target_time )
		{
			isGreaterThanLeft = true;
		}
	}

	bool isGreaterThanRight = false;
	if ( right < transform_history_component.serverTimeBuffer.size() )
	{
		if ( transform_history_component.serverTimeBuffer[ right ] >= target_time )
		{
			isGreaterThanRight = true;
		}
	}
}

static void FindPreviousAndNextTimeIndexes( const ServerTransformHistoryComponent& transform_history_component,
                                            float32 target_time, int32& previous, int32& next )
{
	previous = -1;
	next = -1;

	uint32 firstBiggerIndex = 0;
	bool firstBiggerFound = false;
	for ( ; firstBiggerIndex < transform_history_component.serverTimeBuffer.size(); ++firstBiggerIndex )
	{
		if ( transform_history_component.serverTimeBuffer[ firstBiggerIndex ] >= target_time )
		{
			firstBiggerFound = true;
			break;
		}
	}

	if ( firstBiggerFound )
	{
		if ( firstBiggerIndex == 0 )
		{
			next = firstBiggerIndex;
		}
		else
		{
			next = firstBiggerIndex;
			previous = firstBiggerIndex - 1;
		}
	}
	else
	{
		previous = transform_history_component.serverTimeBuffer.size() - 1;
	}
}

static HistoryEntry GetInterpolatedState( const ServerTransformHistoryComponent& transform_history_component,
                                          int32 previous_index, int32 next_index, float32 server_time )
{
	const float32 previousTime = transform_history_component.serverTimeBuffer[ previous_index ];
	const float32 nextTime = transform_history_component.serverTimeBuffer[ next_index ];
	const float32 t = ( server_time - previousTime ) / ( nextTime - previousTime );
	assert( t >= 0.f && t <= 1.f );

	const HistoryEntry& previousHistoryEntry = transform_history_component.historyBuffer[ previous_index ];
	const HistoryEntry& nextHistoryEntry = transform_history_component.historyBuffer[ next_index ];

	HistoryEntry interpolatedHistoryEntry;
	interpolatedHistoryEntry.position.X(
	    Common::AlgorithmUtils::Lerp( previousHistoryEntry.position.X(), nextHistoryEntry.position.X(), t ) );
	interpolatedHistoryEntry.position.Y(
	    Common::AlgorithmUtils::Lerp( previousHistoryEntry.position.Y(), nextHistoryEntry.position.Y(), t ) );

	interpolatedHistoryEntry.rotationAngle =
	    Common::AlgorithmUtils::Lerp( previousHistoryEntry.rotationAngle, nextHistoryEntry.rotationAngle, t );

	return interpolatedHistoryEntry;
}

static void RollbackEntities( Engine::ECS::World& world, float32 serverTime )
{
	std::vector< Engine::ECS::GameEntity > entitiesWithNetworkComponent =
	    world.GetEntitiesOfType< NetworkEntityComponent >();

	auto it = entitiesWithNetworkComponent.begin();
	for ( ; it != entitiesWithNetworkComponent.end(); ++it )
	{
		const ServerTransformHistoryComponent& transformHistoryComponent =
		    it->GetComponent< ServerTransformHistoryComponent >();

		// Based on the server time we need to rollback, get the closest upper and lower timestamps
		int32 previousIndex = -1;
		int32 nextIndex = -1;
		FindPreviousAndNextTimeIndexes( transformHistoryComponent, serverTime, previousIndex, nextIndex );
		if (nextIndex < 0)
		{
			bool a = true;
		}
		//TODO Investigate hit reg issue that is hitting this assert.
		assert( nextIndex >= 0 );

		Engine::TransformComponentProxy transformComponent( *it );

		// If the server time is older than the oldest timestamp in the buffer, clamp it
		if ( previousIndex == -1 )
		{
			const HistoryEntry& historyEntry = transformHistoryComponent.historyBuffer[ 0 ];
			transformComponent.SetGlobalPosition( historyEntry.position );
			transformComponent.SetGlobalRotationAngle( historyEntry.rotationAngle );
		}
		// Otherwise interpolate between the upper and lower timestamps to be as accurate as possible.
		else
		{
			const HistoryEntry interpolatedHistoryEntry =
			    GetInterpolatedState( transformHistoryComponent, previousIndex, nextIndex, serverTime );

			transformComponent.SetGlobalPosition( interpolatedHistoryEntry.position );
			transformComponent.SetGlobalRotationAngle( interpolatedHistoryEntry.rotationAngle );
		}
	}
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

	// If there are pending shots to process...
	if ( hitRegGlobalComponent.pendingShotEntries.size() > 0 )
	{
		// Save the current state in order to recover it once all shots were processed
		RewindableEntitiesSnapshot snapshot;
		SaveCurrentState( world, snapshot );

		// For each pending shot...
		while ( !hitRegGlobalComponent.pendingShotEntries.empty() )
		{
			const ShotEntry shotEntry = hitRegGlobalComponent.pendingShotEntries.front();
			hitRegGlobalComponent.pendingShotEntries.pop();

			// TODO Instead of discarting invalid shot entries, process them with the maximum allowed rollback time
			// TODO Also consider the client-side delay from remote entity interpolation and not only the latency
			if ( IsShotEntryValid( world, hitRegGlobalComponent, shotEntry ) )
			{
				// Rollback all entities to the shot's server time and perform the shot
				RollbackEntities( world, shotEntry.serverTime );
				// TODO Force an update of all transform hierarchies so the shot doesn't collide with any child entity
				// with a collider that has not been repositioned
				ProcessShotEntry( world, shotEntry );
			}
			else
			{
				LOG_WARNING( "%s: Invalid Shot Entry. Skipping it.", THIS_FUNCTION_NAME );
			}
		}

		// Restore the state to continue with simulation
		RestoreCurrentState( world, snapshot );
	}
}
