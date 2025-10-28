#include "player_state_configuration_utils.h"

#include "numeric_types.h"
#include "asserts.h"

#include "asset_manager/asset_manager.h"
#include "asset_manager/asset_handle.h"

#include "configuration_assets/configuration_asset.h"

namespace PlayerSimulation
{
	PlayerStateConfiguration InitializePlayerConfigFromAsset( const std::string& path,
	                                                          const Engine::AssetManager& asset_manager )
	{
		const Engine::AssetHandle playerConfigAssetHandle =
		    asset_manager.GetAsset( path, Engine::AssetType::CONFIGURATION );
		ASSERT( playerConfigAssetHandle.IsValid(), "Can't load Player Configuration asset." );

		const Engine::ConfigurationAsset* playerConfigAsset = asset_manager.GetRawAsset< Engine::ConfigurationAsset >(
		    playerConfigAssetHandle, Engine::AssetType::CONFIGURATION );

		return PlayerSimulation::PlayerStateConfiguration(
		    playerConfigAsset->GetValue< int32 >( "movementSpeed" ),
		    playerConfigAsset->GetValue< float32 >( "aimingMovementSpeedMultiplier" ),
		    playerConfigAsset->GetValue< int32 >( "fireRatePerSecond" ) );
	}
}
