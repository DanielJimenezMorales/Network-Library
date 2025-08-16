#include "server_player_simulation_events_processor.h"

#include "logger.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/transform_component.h"

#include "server/hit_reg/shot_entry.h"
#include "server/global_components/hit_registration_global_component.h"
#include "server/global_components/server_remote_peer_inputs_global_component.h"

#include "shared/components/network_entity_component.h"

static void OnShotPerformed( Engine::ECS::World& world, const Engine::ECS::GameEntity& player_entity )
{
	const Engine::TransformComponent& playerTransform = player_entity.GetComponent< Engine::TransformComponent >();
	ShotEntry shotEntry;
	shotEntry.position = playerTransform.GetPosition();
	shotEntry.direction = playerTransform.GetForwardVector();
	shotEntry.shooterEntity = player_entity;
	shotEntry.damage = 10; // TODO: Get the damage from the player entity or some configuration

	const ServerRemotePeerInputsGlobalComponent& remotePeerInputsComponent =
	    world.GetGlobalComponent< ServerRemotePeerInputsGlobalComponent >();
	const NetworkEntityComponent& networkEntityComponent = player_entity.GetComponent< NetworkEntityComponent >();

	const RemotePeerInputsStorage& inputsStorage =
	    remotePeerInputsComponent.remotePeerInputs.at( networkEntityComponent.controlledByPeerId );
	shotEntry.serverTime = inputsStorage.lastInputState.serverTime;

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
