#include "configuration_assets_initialization_utils.h"

#include "asset_manager/asset_manager.h"

#include "configuration_assets/configuration_asset_loader.h"

#include "game.h"

namespace Engine
{
	static bool AddConfigurationAssetmanagement( Game& game )
	{
		AssetManager& assetManager = game.GetAssetManager();
		assetManager.RegisterAsset( AssetType::CONFIGURATION, new ConfigurationAssetLoader() );
		return true;
	}

	bool AddConfigurationAssetsToWorld( Game& game )
	{
		bool result = AddConfigurationAssetmanagement( game );
		if ( !result )
		{
			return false;
		}

		return true;
	}
}
