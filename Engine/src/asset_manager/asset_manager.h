#pragma once
#include "numeric_types.h"

#include "asset_manager/asset_type.h"
#include "asset_manager/asset_handle.h"

#include <string>
#include <unordered_map>

namespace Engine
{
	class AssetStorage;
	class IAssetLoader;
	class Asset;

	class AssetManager
	{
		public:
			AssetManager() = default;

			bool RegisterAsset( AssetType type, IAssetLoader* loader );

			const AssetHandle GetAsset( const std::string& path, AssetType type ) const;

			template < typename T >
			const T* GetRawAsset( const AssetHandle& handle, AssetType type ) const;

		private:
			bool IsAssetRegistered( AssetType type ) const;

			AssetStorage* TryGetAssetStorage( AssetType type ) const;
			IAssetLoader* TryGetAssetLoader( AssetType type ) const;

			const Asset* GetRawAssetInternal( const AssetHandle& handle, AssetType type ) const;

			std::unordered_map< AssetType, IAssetLoader* > _loaders;
			std::unordered_map< AssetType, AssetStorage* > _storages;
	};

	template < typename T >
	inline const T* AssetManager::GetRawAsset( const AssetHandle& handle, AssetType type ) const
	{
		const Asset* asset = GetRawAssetInternal( handle, type );
		if ( asset != nullptr )
		{
			return static_cast< const T* >( asset );
		}

		return nullptr;
	}
} // namespace Engine