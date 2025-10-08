#pragma once
#include "asset_manager/asset_type.h"
#include "asset_manager/asset_handle.h"

#include <string>
#include <unordered_map>

namespace Engine
{
	class Asset;

	class AssetStorage
	{
		public:
			AssetStorage( AssetType type );

			AssetHandle StoreAsset( Asset* asset );
			const AssetHandle GetAsset( const std::string& path ) const;
			const Asset* GetRawAsset( const AssetHandle& handle ) const;

		private:
			bool IsAssetCached( const std::string& path ) const;
			Asset* GetCachedAsset( const std::string& path ) const;

			std::unordered_map< std::string, Asset* > _pathToCachedAssetMap;
			AssetType _type;
	};
}
