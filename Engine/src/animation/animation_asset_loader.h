#pragma once
#include "asset_manager/i_asset_loader.h"
#include "asset_manager/asset_type.h"

namespace Engine
{
	class AnimationAssetLoader : public IAssetLoader
	{
		public:
			AnimationAssetLoader() = default;
			~AnimationAssetLoader() {};

			AssetType GetType() const override { return AssetType::ANIMATION; }
			Asset* Load( const std::string& path ) override;
	};

}
