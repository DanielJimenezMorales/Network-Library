#pragma once
#include "asset_manager/i_asset_loader.h"
#include "asset_manager/asset_type.h"

namespace Engine
{
	class ConfigurationAssetLoader : public IAssetLoader
	{
		public:
			ConfigurationAssetLoader() = default;
			~ConfigurationAssetLoader() {};

			AssetType GetType() const override { return AssetType::CONFIGURATION; }
			Asset* Load( const std::string& path ) override;
	};

}
