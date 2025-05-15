#include "client_local_player_server_reconciliator_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/client_side_prediction_component.h"
#include "components/player_controller_component.h"

#include "global_components/network_peer_global_component.h"

#include "player_simulation/player_state_simulator.h"
#include "player_simulation/player_state_utils.h"

#include "logger.h"

ClientLocalPlayerServerReconciliatorSystem::ClientLocalPlayerServerReconciliatorSystem()
    : ECS::ISimpleSystem()
{
}

static PlayerState GetMostRecentPendingServerPlayerState(
    const ClientSidePredictionComponent& client_side_prediction_component )
{
	PlayerState result;
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

void ClientLocalPlayerServerReconciliatorSystem::Execute( ECS::World& world, float32 elapsed_time )
{
	const NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	if ( networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected )
	{
		return;
	}

	std::vector< ECS::GameEntity > localPlayers = world.GetEntitiesOfType< ClientSidePredictionComponent >();
	for ( auto it = localPlayers.begin(); it != localPlayers.end(); ++it )
	{
		ClientSidePredictionComponent& clientSidePredictionComponent =
		    it->GetComponent< ClientSidePredictionComponent >();

		if ( clientSidePredictionComponent.isPendingPlayerStateFromServer )
		{
			const PlayerState& playerStateFromServer =
			    GetMostRecentPendingServerPlayerState( clientSidePredictionComponent );

			if ( clientSidePredictionComponent.lastTickProcessedFromServer < playerStateFromServer.tick )
			{
				const uint32 slotIndex =
				    clientSidePredictionComponent.ConvertTickToBufferSlotIndex( playerStateFromServer.tick );

				if ( playerStateFromServer.tick ==
				     clientSidePredictionComponent.resultedPlayerStatesBuffer[ slotIndex ].tick )
				{
					const PlayerState& predictedState =
					    clientSidePredictionComponent.resultedPlayerStatesBuffer[ slotIndex ];

					if ( predictedState != playerStateFromServer )
					{
						clientSidePredictionComponent.resultedPlayerStatesBuffer[ slotIndex ] = playerStateFromServer;

						PlayerStateSimulator playerStateSimulator;

						const uint32 lastTick = networkPeerComponent.peer->GetCurrentTick();
						uint32 currentTick = playerStateFromServer.tick + 1;
						PlayerState currentPlayerState = playerStateFromServer;
						const PlayerControllerComponent& playerControllerComponent =
						    it->GetComponent< PlayerControllerComponent >();
						const PlayerStateConfiguration& playerStateConfiguration =
						    playerControllerComponent.stateConfiguration;

						while ( currentTick != lastTick )
						{
							const uint32 currentSlotIndex =
							    clientSidePredictionComponent.ConvertTickToBufferSlotIndex( currentTick );

							const InputState& inputState =
							    clientSidePredictionComponent.inputStatesBuffer[ currentSlotIndex ];
							const float32 elapsedTime =
							    clientSidePredictionComponent.elapsedTimeBuffer[ currentSlotIndex ];
							const PlayerState resultedPlayerState = playerStateSimulator.Simulate(
							    inputState, currentPlayerState, playerStateConfiguration, elapsedTime );

							clientSidePredictionComponent.resultedPlayerStatesBuffer[ currentSlotIndex ] =
							    resultedPlayerState;
							currentPlayerState = resultedPlayerState;

							++currentTick;
						}

						ApplyPlayerStateToPlayerEntity( *it, currentPlayerState );
					}
				}
			}

			clientSidePredictionComponent.lastTickProcessedFromServer = playerStateFromServer.tick;
			clientSidePredictionComponent.playerStatesReceivedFromServer.clear();
			clientSidePredictionComponent.isPendingPlayerStateFromServer = false;
		}
	}
}
