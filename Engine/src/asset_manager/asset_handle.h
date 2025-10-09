#pragma once
#include "numeric_types.h"

#include "asset_manager/asset_type.h"

#include <string>

namespace Engine
{
	struct AssetHandle
	{
		public:
			AssetHandle( const std::string& key, AssetType type )
			    : key( key )
			    , _type( type ) {};

			static AssetHandle GetInvalid() { return AssetHandle( "\0", AssetType::TEXTURE ); }

			bool IsValid() const { return strcmp( key.c_str(), "\0" ) != 0; }
			AssetType GetType() const { return _type; }

		private:
			std::string key;
			AssetType _type;

			friend class AssetStorage;
	};
} // namespace Engine
