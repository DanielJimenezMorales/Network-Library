#include "asset_storage.h"
#include "asset_manager/asset.h"

#include "logger.h"

#include <cassert>

namespace Engine
{
	AssetStorage::AssetStorage( AssetType type )
	    : _type( type )
	    , _pathToCachedAssetMap()
	{
	}

	AssetHandle AssetStorage::StoreAsset( Asset* asset )
	{
		assert( asset != nullptr );
		assert( asset->GetType() == _type );

		const std::string& path = asset->GetPath();
		if ( IsAssetCached( path ) )
		{
			LOG_WARNING( "Asset already cached. Texture path: %s", path.c_str() );
			return AssetHandle::GetInvalid();
		}

		_pathToCachedAssetMap.emplace( path, asset );
		return AssetHandle( path, _type );
	}

	const AssetHandle AssetStorage::GetAsset( const std::string& path ) const
	{
		return AssetHandle::GetInvalid();
	}

	const Asset* AssetStorage::GetRawAsset( const AssetHandle& handle ) const
	{
		assert( handle.IsValid() );
		assert( handle.GetType() == _type );

		return GetCachedAsset( handle.key.c_str() );
	}

	bool AssetStorage::IsAssetCached( const std::string& path ) const
	{
		return GetCachedAsset( path ) != nullptr;
	}

	Asset* AssetStorage::GetCachedAsset( const std::string& path ) const
	{
		Asset* asset = nullptr;
		auto assetFound = _pathToCachedAssetMap.find( path );
		if ( assetFound != _pathToCachedAssetMap.end() )
		{
			asset = assetFound->second;
		}

		return asset;
	}
} // namespace Engine
