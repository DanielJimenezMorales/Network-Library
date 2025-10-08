#pragma once
#include "asset_manager/asset_type.h"

#include <string>

namespace Engine
{
	class Asset
	{
		public:
			Asset( const std::string& path )
			    : _path( path ) {};

			virtual ~Asset() {}

			const std::string& GetPath() const { return _path; }
			virtual AssetType GetType() const = 0;

		private:
			std::string _path;
	};
}
