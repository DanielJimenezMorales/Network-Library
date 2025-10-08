#include "client_local_player_server_reconciliator_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "shared/components/player_controller_component.h"

#include "client/components/client_side_prediction_component.h"

#include "shared/global_components/network_peer_global_component.h"

#include "shared/player_simulation/player_state_simulator.h"
#include "shared/player_simulation/player_state_utils.h"

#include "logger.h"

ClientLocalPlayerServerReconciliatorSystem::ClientLocalPlayerServerReconciliatorSystem()
    : Engine::ECS::ISimpleSystem()
{
}

static bool IsAReconciliationNeeded( const ClientSidePredictionComponent& prediction_component,
                                     const PlayerSimulation::PlayerState& state_from_server )
{
	bool result = false;

	// If the client has already processed a newer one we skip it
	if ( prediction_component.lastTickProcessedFromServer < state_from_server.tick )
	{
		// Get the cyclic buffer index based on the tick
		const uint32 slotIndex = prediction_component.ConvertTickToBufferSlotIndex( state_from_server.tick );

		// If the tick stored in the buffer slot index is different it means that it has been overwritten
		if ( state_from_server.tick == prediction_component.resultedPlayerStatesBuffer[ slotIndex ].tick )
		{
			// Check if state received from server is different from the one predicted locally. In that case we
			// need to reconciliate.
			const PlayerSimulation::PlayerState& predictedState =
			    prediction_component.resultedPlayerStatesBuffer[ slotIndex ];
			if ( predictedState != state_from_server )
			{
				result = true;
			}
		}
		else
		{
			LOG_WARNING( "The prediction buffer slot has been overwritten by a newer state. As a consequence, "
			             "the reconciliation with the server's state can't be performed. Increase buffer size "
			             "or reduce RTT. Prediction buffer tick: %u, Server's state tick: %u",
			             prediction_component.resultedPlayerStatesBuffer[ slotIndex ].tick, state_from_server.tick );
		}
	}

	return result;
}

static void ReconciliateWithServer( Engine::ECS::GameEntity& entity,
                                    ClientSidePredictionComponent& prediction_component,
                                    const PlayerSimulation::PlayerState& state_from_server,
                                    const NetworkPeerGlobalComponent& network_peer )
{
	LOG_WARNING( "Reconciliating with server. Server tick: %u", state_from_server.tick );
	IsAReconciliationNeeded(prediction_component, state_from_server);
	    // Overwrite the predicted state with the one received from server
	    const uint32 slotIndexToReconciliate =
	        prediction_component.ConvertTickToBufferSlotIndex( state_from_server.tick );
	prediction_component.resultedPlayerStatesBuffer[ slotIndexToReconciliate ] = state_from_server;

	// Get all necessary data for passing it to the state simulator
	const PlayerControllerComponent& playerControllerComponent = entity.GetComponent< PlayerControllerComponent >();
	const PlayerSimulation::PlayerStateConfiguration& playerStateConfiguration =
	    playerControllerComponent.stateConfiguration;
	PlayerSimulation::PlayerStateSimulator playerStateSimulator;

	PlayerSimulation::PlayerState currentPlayerState = state_from_server;
	uint32 currentTick = state_from_server.tick + 1;
	const uint32 lastTick = network_peer.peer->GetCurrentTick();

	// Re-simulate the entity logic with the corrected simulation state from server until reaching the current tick.
	while ( currentTick != lastTick )
	{
		const uint32 currentSlotIndex = prediction_component.ConvertTickToBufferSlotIndex( currentTick );

		const InputState& inputState = prediction_component.inputStatesBuffer[ currentSlotIndex ];
		const float32 elapsedTime = prediction_component.elapsedTimeBuffer[ currentSlotIndex ];
		const PlayerSimulation::PlayerState resultedPlayerState =
		    playerStateSimulator.Simulate( inputState, currentPlayerState, playerStateConfiguration, elapsedTime );

		prediction_component.resultedPlayerStatesBuffer[ currentSlotIndex ] = resultedPlayerState;
		currentPlayerState = resultedPlayerState;

		++currentTick;
	}

	// Apply the current simulation state to the entity
	ApplyPlayerStateToPlayerEntity( entity, currentPlayerState );
}

static PlayerSimulation::PlayerState GetMostRecentPendingServerPlayerState(
    const ClientSidePredictionComponent& client_side_prediction_component )
{
	PlayerSimulation::PlayerState result;
	result.ZeroOut();

	auto cit = client_side_prediction_component.playerStatesReceivedFromServer.cbegin();
	for ( ; cit != client_side_prediction_component.playerStatesReceivedFromServer.cend(); ++cit )
	{
		if ( result.tick < cit->tick )
		{
			result = *cit;
		}
	}

	return result;
}

static void EvaluateReconciliation( Engine::ECS::GameEntity& entity, const NetworkPeerGlobalComponent& network_peer )
{
	ClientSidePredictionComponent& clientSidePredictionComponent =
	    entity.GetComponent< ClientSidePredictionComponent >();

	// Check if we have any pending simulation state from the server to process
	if ( clientSidePredictionComponent.isPendingPlayerStateFromServer )
	{
		// Get the state received from server (in case we have more than one we'll only care about the most recent)
		const PlayerSimulation::PlayerState& playerStateFromServer =
		    GetMostRecentPendingServerPlayerState( clientSidePredictionComponent );

		if ( IsAReconciliationNeeded( clientSidePredictionComponent, playerStateFromServer ) )
		{
			ReconciliateWithServer( entity, clientSidePredictionComponent, playerStateFromServer, network_peer );
		}

		clientSidePredictionComponent.lastTickProcessedFromServer = playerStateFromServer.tick;
		clientSidePredictionComponent.playerStatesReceivedFromServer.clear();
		clientSidePredictionComponent.isPendingPlayerStateFromServer = false;
	}
}

void ClientLocalPlayerServerReconciliatorSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	if ( networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected )
	{
		return;
	}

	std::vector< Engine::ECS::GameEntity > localPredictedEntities =
	    world.GetEntitiesOfType< ClientSidePredictionComponent >();
	for ( auto it = localPredictedEntities.begin(); it != localPredictedEntities.end(); ++it )
	{
		EvaluateReconciliation( *it, networkPeerComponent );
	}
}
