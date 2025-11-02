#pragma once
#include "shared/player_simulation/player_state_configuration.h"

#include <string>

namespace Engine
{
	class AssetManager;
}

namespace PlayerSimulation
{
	PlayerStateConfiguration InitializePlayerConfigFromAsset( const std::string& path,
	                                                           const Engine::AssetManager& asset_manager );
}