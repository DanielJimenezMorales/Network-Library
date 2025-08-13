#pragma once

#include "shared/player_simulation/player_state.h"

/// <summary>
/// This component is only server-side and it's used to store the last player state simulated in order to serialize it
/// and send it to the client. Without this component, we would be loosing track of variables such as the player state
/// tick
/// </summary>
struct ServerPlayerStateStorageComponent
{
		ServerPlayerStateStorageComponent() {}

		PlayerSimulation::PlayerState lastPlayerStateSimulated;
};
