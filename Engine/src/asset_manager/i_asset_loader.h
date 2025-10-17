#pragma once
#include "asset_manager/asset_type.h"

#include <string>

namespace Engine
{
	class Asset;

	class IAssetLoader
	{
		public:
			virtual ~IAssetLoader() {};

			virtual AssetType GetType() const = 0;
			virtual Asset* Load( const std::string& path ) = 0;
	};
}
