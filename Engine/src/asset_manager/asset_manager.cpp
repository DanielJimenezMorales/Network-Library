#include "asset_manager.h"

#include "logger.h"

#include "asset_manager/asset_storage.h"
#include "asset_manager/i_asset_loader.h"
#include "asset_manager/asset.h"

#include <cassert>

namespace Engine
{
	bool AssetManager::RegisterAsset( AssetType type, IAssetLoader* loader )
	{
		bool result = false;
		if ( !IsAssetRegistered( type ) )
		{
			_loaders[ type ] = loader;
			_storages[ type ] = new AssetStorage( type );
			result = true;
		}
		else
		{
			LOG_ERROR( "[%s] Asset type %d is already registered.", THIS_FUNCTION_NAME, static_cast< uint32 >( type ) );
		}

		return result;
	}

	const AssetHandle AssetManager::GetAsset( const std::string& path, AssetType type ) const
	{
		AssetHandle handle = AssetHandle::GetInvalid();

		AssetStorage* storage = TryGetAssetStorage( type );
		if ( storage != nullptr )
		{
			handle = storage->GetAsset( path );
			if ( !handle.IsValid() )
			{
				IAssetLoader* loader = TryGetAssetLoader( type );
				if ( loader != nullptr )
				{
					Asset* asset = loader->Load( path );
					if ( asset != nullptr )
					{
						handle = storage->StoreAsset( asset );
						assert( handle.IsValid() );
					}
					else
					{
						LOG_ERROR( "[%s] Error loading asset of type %d at path %s", THIS_FUNCTION_NAME,
						           static_cast< uint32 >( type ), path.c_str() );
					}
				}
				else
				{
					LOG_ERROR( "[%s] No loader found for asset type %d. Please register an Asset Loader.",
					           THIS_FUNCTION_NAME, static_cast< uint32 >( type ) );
				}
			}
		}
		else
		{
			LOG_ERROR( "[%s] No storage found for asset type %d. Please register an Asset Storage.", THIS_FUNCTION_NAME,
			           static_cast< uint32 >( type ) );
		}

		return handle;
	}

	bool AssetManager::IsAssetRegistered( AssetType type ) const
	{
		return ( _loaders.find( type ) != _loaders.end() || _storages.find( type ) != _storages.end() );
	}

	AssetStorage* AssetManager::TryGetAssetStorage( AssetType type ) const
	{
		AssetStorage* result = nullptr;
		auto it = _storages.find( type );
		if ( it != _storages.end() )
		{
			result = it->second;
		}

		return result;
	}

	IAssetLoader* AssetManager::TryGetAssetLoader( AssetType type ) const
	{
		IAssetLoader* result = nullptr;
		auto it = _loaders.find( type );
		if ( it != _loaders.end() )
		{
			result = it->second;
		}

		return result;
	}

	const Asset* AssetManager::GetRawAssetInternal( const AssetHandle& handle, AssetType type ) const
	{
		if ( handle.IsValid() && handle.GetType() == type )
		{
			const AssetStorage* storage = TryGetAssetStorage( type );
			if ( storage != nullptr )
			{
				const Asset* asset = storage->GetRawAsset( handle );
				return asset;
			}
			else
			{
				LOG_ERROR( "[%s] No storage found for asset type %d. Please register an Asset Storage.",
				           THIS_FUNCTION_NAME, static_cast< uint32 >( type ) );
			}
		}
		else
		{
			LOG_ERROR(
			    "[%s] Invalid asset handle or mismatched asset type. Handle valid: %d, Handle type: %d, Requested "
			    "type: %d",
			    THIS_FUNCTION_NAME, handle.IsValid() ? 1 : 0, static_cast< uint32 >( handle.GetType() ),
			    static_cast< uint32 >( type ) );
		}

		return nullptr;
	}
} // namespace Engine
