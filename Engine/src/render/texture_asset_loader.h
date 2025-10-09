#pragma once
#include "asset_manager/i_asset_loader.h"
#include "asset_manager/asset_type.h"

struct SDL_Renderer;

namespace Engine
{
	class TextureAssetLoader : public IAssetLoader
	{
		public:
			TextureAssetLoader( SDL_Renderer* renderer );
			~TextureAssetLoader() {};

			AssetType GetType() const override { return AssetType::TEXTURE; }
			Asset* Load( const std::string& path ) override;

		private:
			SDL_Renderer* _renderer;
	};
}
