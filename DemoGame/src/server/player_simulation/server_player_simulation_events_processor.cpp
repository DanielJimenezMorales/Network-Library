#include "server_player_simulation_events_processor.h"

#include "logger.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/transform_component.h"

#include "server/hit_reg/shot_entry.h"
#include "server/global_components/hit_registration_global_component.h"

#include "shared/components/network_entity_component.h"
#include "shared/global_components/network_peer_global_component.h"
#include "shared/InputState.h"

static void OnShotPerformed( Engine::ECS::World& world, const Engine::ECS::GameEntity& player_entity )
{
	const Engine::TransformComponent& playerTransform = player_entity.GetComponent< Engine::TransformComponent >();
	ShotEntry shotEntry;
	shotEntry.position = playerTransform.GetPosition();
	shotEntry.direction = playerTransform.GetForwardVector();
	shotEntry.shooterEntity = player_entity;
	shotEntry.damage = 10; // TODO: Get the damage from the player entity or some configuration

	// Get the last input simulated
	const NetworkPeerGlobalComponent& networkPeerGlobalComponent =
	    world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	const NetworkEntityComponent& networkEntityComponent = player_entity.GetComponent< NetworkEntityComponent >();

	const NetLib::Server* server = networkPeerGlobalComponent.GetPeerAsServer();
	const NetLib::IInputState* baseLastInputState =
	    server->GetLastInputPoppedFromRemotePeer( networkEntityComponent.controlledByPeerId );
	assert( baseLastInputState != nullptr );

	const InputState* lastInputState = static_cast< const InputState* >( baseLastInputState );
	shotEntry.serverTime = lastInputState->serverTime;

	HitRegistrationGlobalComponent& hitRegGlobalComponent =
	    world.GetGlobalComponent< HitRegistrationGlobalComponent >();
	hitRegGlobalComponent.pendingShotEntries.push( shotEntry );
}

bool ServerPlayerSimulationEventsProcessor::ProcessEvent( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
                                                          PlayerSimulation::EventType type )
{
	bool result = true;

	if ( type == PlayerSimulation::ON_SHOT_PERFORMED )
	{
		OnShotPerformed( world, entity );
	}
	else
	{
		result = false;
	}

	return result;
}
